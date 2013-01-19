/*****************************************************************************
*                                                                           *
* Tiger Mapping and Routing Server  (TMRS)                                  *
*                                                                           *
* Copyright (C) 2003 Sumit Birla <sbirla@users.sourceforge.net>             *
*                                                                           *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License for more details.                              *
*                                                                           *
* You should have received a copy of the GNU General Public License         *
* along with this program; if not, write to the Free Software               *
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA  *
*                                                                           *
*****************************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "shapefil.h"
#include "../tmrs_structs.h"


// Function prototypes
void process_file(char *filename);
void process_polylines(SHPHandle hSHP, DBFHandle hDBF, int numRecs);
void process_polygons(SHPHandle hSHP, DBFHandle hDBF, int numRecs);

// global variables (bad idea, I know)
FILE *fp_segments, *fp_chains, *fp_polygons;
struct _StreetName *street;
int num_segments, num_chains, num_polygons, num_names;
int name_index_start, allocated_mem;


/* program entry point */
int main(int argc, char **argv)
{
    int optchar, len;
    char filename[256];
    char *data_dir = "./";   // default directory
    FILE *fp_names;
    DIR * dirp;
    struct dirent * dp;

    // parse options passed-in on the command line 
    while ((optchar = getopt (argc, argv, "d:")) != -1)
    {
        switch (optchar)
        {
        case 'd':
            data_dir = (char *) strdup (optarg);
            break;

        default:
        case '?':
            printf ("Usage: %s [-d datadir]\n\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    // initialize variables 
    num_segments = 0;
    num_chains = 0;
    num_polygons = 0;
    num_names = 0;
    allocated_mem = 0;
    name_index_start = 0;

    // Open tmrs data files for writing 
    fp_segments= fopen("segments.dat", "w" );
    fp_chains = fopen("chains.dat", "w");
    fp_polygons = fopen("polygons.dat", "w");

    // write the initial count for chains and polygons
    fwrite(&num_chains, sizeof(int), 1, fp_chains);
    fwrite(&num_polygons, sizeof(int), 1, fp_polygons);

    // open the data directory 
    dirp = opendir(data_dir);
    if (dirp == NULL)
    {
        perror(data_dir);
        exit(EXIT_FAILURE);
    }

    // find all Shapefiles in the directory and process them. 
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
        len = strlen(dp->d_name);
        if ((len > 4) && !strcmp(&dp->d_name[len-3], "shp"))
        {
            dp->d_name[len-3] = '\0';   // copy without .shp
            sprintf(filename, "%s/%s", data_dir, dp->d_name);  // prepend dir
            process_file(filename);
        }
    }

    closedir (dirp);
    fclose(fp_segments);

    // print the statistics
    printf( "\nSummary\n");
    printf( "------------------------------------------------\n");
    printf( "Number of segments   \t= %d, \t%d kB\n", num_segments, 
        num_segments*sizeof(struct _RoadSegment)/1024);
    printf( "Number of shape entries = %d,\t%d kB\n", num_chains, 
        ftell(fp_chains)/1024 );
    printf( "Number of polygons   \t= %d, \t%d kB\n", num_polygons, 
        ftell(fp_polygons)/1024 );
    printf( "Unique street names \t= %d,\t%d kB\n", num_names, 
        num_names*sizeof(struct _StreetName)/1024);
    printf( "------------------------------------------------\n\n");

    // update the chain count in the output file before closing
    fseek(fp_chains, 0, SEEK_SET);
    fwrite(&num_chains, sizeof(int), 1, fp_chains);
    fclose(fp_chains);

    // update the polygon count in the output file before closing
    fseek(fp_polygons, 0, SEEK_SET);
    fwrite(&num_polygons, sizeof(int), 1, fp_polygons);
    fclose(fp_polygons);

    // write the street names out to file
    fp_names = fopen("names.dat", "w");
    fwrite(street, num_names, sizeof(struct _StreetName), fp_names);
    fclose(fp_names);
    free(street);

    return EXIT_SUCCESS;
}


/* This function extract data from one set of shapefiles at a time. */
void process_file(char *filename)
{
    SHPHandle hSHP;
    DBFHandle hDBF;
    int nEntities, nShapeType;
    double padfMinBound[4], padfMaxBound[4];

    // open the shapefile 
    hSHP = SHPOpen(filename, "rb");
    if (hSHP == NULL) {
        perror(filename);
        return;
    }

    // verify the shapefile contains either arcs (polylines) or polygons 
    SHPGetInfo(hSHP, &nEntities, &nShapeType, padfMinBound, padfMaxBound);
    printf("%s: %d records, ShapeType %d, Processing: ", filename, 
        nEntities, nShapeType);

    if ((nShapeType != SHPT_ARC) && (nShapeType != SHPT_POLYGON)) 
    {
        printf("%s.shp does not contain arcs or polygons. Skipping...\n", 
            filename);
        SHPClose(hSHP);
        return;
    }

    // open the corresponding DBF file 
    hDBF = DBFOpen(filename, "rb");
    if (hDBF == NULL) {
        SHPClose(hSHP);
        perror(filename);
        return;
    }

    // call the appropriate function to extract data 
    if (nShapeType == SHPT_ARC) 
        process_polylines(hSHP, hDBF, nEntities);
    else if (nShapeType == SHPT_POLYGON)
        process_polygons(hSHP, hDBF, nEntities);

    // cleanup 
    DBFClose(hDBF);
    SHPClose(hSHP);
}


/* Function to process shapefile with polylines (streets) */
void process_polylines(SHPHandle hSHP, DBFHandle hDBF, int numRecs)
{
    int iStreetPrefix, iStreetName, iStreetType, iStreetSuffix;
    int iStartAddressLeft, iStartAddressRight;
    int iEndAddressLeft, iEndAddressRight;
    int iCFCC, i, percent_complete, prev_percent = -1;
    char *cfcc, name[39];
    struct _RoadSegment rec;
    SHPObject *pShape; 

    // verify that this is an ESRI Tiger dataset 
    iCFCC = DBFGetFieldIndex(hDBF, "CFCC");
    iStreetPrefix = DBFGetFieldIndex(hDBF, "FEDIRP");
    iStreetName = DBFGetFieldIndex(hDBF, "FENAME");
    iStreetType = DBFGetFieldIndex(hDBF, "FETYPE");
    iStreetSuffix = DBFGetFieldIndex(hDBF, "FEDIRS");
    iStartAddressLeft = DBFGetFieldIndex(hDBF, "FRADDL");
    iEndAddressLeft = DBFGetFieldIndex(hDBF, "TOADDL");
    iStartAddressRight = DBFGetFieldIndex(hDBF, "FRADDR");
    iEndAddressRight = DBFGetFieldIndex(hDBF, "TOADDR");

    if ((iCFCC == -1) || 
        (iStreetPrefix == -1)  || (iStreetName == -1) ||
        (iStreetType == -1) || (iStreetSuffix == -1) ||
        (iStartAddressLeft == -1) || (iEndAddressLeft == -1) ||
        (iStartAddressRight == -1) || (iEndAddressRight == -1))
    {
        printf("Attributes to not correspond to TIGER fields.\n");
        return;
    }

    // read in data from shape file and write to TMRS data files 
    for (i = 0; i < numRecs; i++)
    {
        memset(&rec, 0, sizeof(rec));
        pShape = SHPReadObject(hSHP, i);

        // find out the type of polygon (constants defined in tmrs_structs.h
        cfcc = (char *)DBFReadStringAttribute(hDBF, i, iCFCC);
        rec.RoadClass = atoi(&cfcc[1]);

        rec.StartAddressLeft = DBFReadIntegerAttribute(hDBF, i, iStartAddressLeft);
        rec.StartAddressRight = DBFReadIntegerAttribute(hDBF, i, iStartAddressRight);
        rec.EndAddressLeft = DBFReadIntegerAttribute(hDBF, i, iEndAddressLeft);
        rec.EndAddressRight = DBFReadIntegerAttribute(hDBF, i, iEndAddressRight);

        rec.StartPoint.Latitude = pShape->padfY[0] * 1000000.0;
        rec.StartPoint.Longitude = pShape->padfX[0] * 1000000.0;
        rec.EndPoint.Latitude = pShape->padfY[pShape->nVertices-1] * 1000000.0;
        rec.EndPoint.Longitude = pShape->padfX[pShape->nVertices-1] * 1000000.0;;

        strcpy(name, DBFReadStringAttribute(hDBF, i, iStreetPrefix));
        strcat(name, DBFReadStringAttribute(hDBF, i, iStreetName));
        strcat(name, DBFReadStringAttribute(hDBF, i, iStreetType));
        strcat(name, DBFReadStringAttribute(hDBF, i, iStreetSuffix));

        rec.StreetIndex = get_name_index(name);
        if (pShape->nVertices > 2)
            rec.ShapeIndex = get_chain_index(pShape);
        else
            rec.ShapeIndex = -1;

        fwrite(&rec, sizeof(rec), 1, fp_segments);
        ++num_segments;
	SHPDestroyObject(pShape);
        
	/* progress indicator */
        percent_complete = 100 * i / numRecs;
        if (percent_complete > prev_percent)
        {
            printf(" %.2d%\b\b\b\b", percent_complete);
            fflush(stdout);
            prev_percent = percent_complete;
        }
    }

    // set the starting index for the next set of records 
    name_index_start = num_names;
    printf("Complete\n");
}


/* Function to process shapefile with polygons */
void process_polygons(SHPHandle hSHP, DBFHandle hDBF, int numRecs)
{
    int i, j, k, iCFCC, iLandName, start_index, end_index;
    int num_points, percent_complete, prev_percent = -1, p;
    char *cfcc, type, *name;
    struct _Coordinates c;
    SHPObject *pShape;

    // Verify that this is ESRI Tiger data by looking at attributes in DBF 
    iCFCC = DBFGetFieldIndex(hDBF, "CFCC");
    iLandName = DBFGetFieldIndex(hDBF, "LANDNAME");

    if ((iCFCC == -1) || (iLandName == -1))
    {
        printf("Attributes to not correspond to TIGER fields.\n");
        return;
    }

    // iterate through the records in the shapefile 
    for (i = 0; i < numRecs; i++)
    {
        // find out the type of polygon (constants defined in tmrs_structs.h)
        cfcc = (char *)DBFReadStringAttribute(hDBF, i, iCFCC);
        if (cfcc[0] == 'H') 
            type = POLYGON_WATER;
        else if (cfcc[0] == 'D') {
            p = atoi(&cfcc[1]);
            if ((p == 10) || (p == 51))
                type = POLYGON_AIRPORT;
            else if ((p == 83) || (p == 84) || (p == 85))
                type = POLYGON_PARK;
            else
                continue;
        } 
        else    // uninteresting polygon,  Skip
            continue;

        // read on the shape entry
        pShape = SHPReadObject(hSHP, i);
        name = (char *)DBFReadStringAttribute(hDBF, i, iLandName);

        // each shape record may have multiple parts 
        for (j = 0; j < pShape->nParts; j++)
        {
            start_index = pShape->panPartStart[j];
            if (j == pShape->nParts-1)
                end_index = pShape->nVertices-1;
            else
                end_index = pShape->panPartStart[j+1]-1;

            // write polygon type and name
            fwrite(&type, sizeof(char), 1, fp_polygons);
            fwrite(name, 30, 1, fp_polygons);

            // write the number of points which make up this polygon 
            num_points = end_index - start_index + 1;
            fwrite(&num_points, sizeof(int), 1, fp_polygons);

            // write each point out to file 
            for (k = start_index; k <= end_index; k++)
            {
                c.Latitude = pShape->padfY[k] * 1000000.0;
                c.Longitude = pShape->padfX[k] * 1000000.0;
                fwrite(&c, sizeof(c), 1, fp_polygons);
            }

            ++num_polygons;
        }

	// free up memory
	SHPDestroyObject(pShape);

        // progress indicator 
        percent_complete = 100 * i / numRecs;
        if (percent_complete > prev_percent)
        {
            printf(" %.2d%\b\b\b\b", percent_complete);
            fflush(stdout);
            prev_percent = percent_complete;
        }
    }

    printf("Complete\n");
}


/**
* This method return the index in the Address list where the specified street 
* address is located.  If not already present in the list, it is added.
*
* Returns the street index.
*/
int get_name_index(char *name)
{
    int i;

    // check to see if street is already in array
    for (i = name_index_start; i < num_names; i++)
    {
        if (!memcmp(name, &street[i], sizeof(struct _StreetName)))
            return i;
    }

    // if we reached here, the street does not exist in array, so add it.
    ++num_names;

    // if not enough memory has been allocated..
    if (num_names > allocated_mem)
    {
        allocated_mem += 1000;  // allocated in chunks of 1000
        street = (struct _StreetName *) realloc( street, allocated_mem * 
            sizeof(struct _StreetName) );
    }

    // add the new street into array
    memcpy(&street[num_names-1], name, sizeof(struct _StreetName));

    return num_names-1;
}


/**
* This method writes all the points in a polyline excluding the start and end
* points to file.
* 
* Returns the index at which it is located.
*/
int get_chain_index(SHPObject *pShape)
{
    int i, num_points;
    struct _Coordinates c;

    num_points = pShape->nVertices - 2;
    fwrite(&num_points, sizeof(int), 1, fp_chains);

    for (i = 1; i < pShape->nVertices - 1; i++)
    {
        c.Latitude = pShape->padfY[i] * 1000000.0;
        c.Longitude = pShape->padfX[i] * 1000000.0;

        fwrite(&c, sizeof(c), 1, fp_chains);
    }

    ++num_chains;

    return num_chains -1;
}
