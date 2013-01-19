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


#include <stdio.h>
#include "tiger.h"
#include "../tmrs_structs.h"
#include "tmrs_extract.h"


// Function prototypes
int get_street_index(char *name); 
int get_shape_index(int tlid, struct _Chains *, FILE *);


/**
* The bulk of the work is done in this function.  It parses an RT1 file which 
* contains street segments information. It outputs to segments.dat, names.dat 
* and chains.dat. These .dat files are created and maintained by the main() 
* function.  process_rt1 simply appends to them.  This allows use to compress
* multiple counties into one set of data files.
*/ 
void process_rt1(char *filename, struct _Chains *chains, 
                 FILE *fp_segments, FILE *fp_chains)
{
    FILE *fp_in;
    char str[32];
    int i, length, numRecs, street_index;
    int percent_complete, prev_percent =0;
    int tlid;
    struct _RecordType1 rec1;
    struct _RoadSegment segment;

    printf("\nProcessing: %s --> segments/names/chains\n", filename);
    printf("-----------------------------------------------\n");

    fp_in = fopen( filename, "r" ) ;

    //goto end of file
    fseek( fp_in, 0, SEEK_END );

    //get length of file
    length = ftell( fp_in );
    numRecs = length/sizeof(RecordType1);
    printf("Length of input file \t= %d bytes\n", length); 
    printf("Number of records \t= %d\n", numRecs);

    //go back to beginning of file
    fseek( fp_in, 0, SEEK_SET );

    for (i=0; i < numRecs; i++)
    {
        fread(&rec1, sizeof(RecordType1), 1, fp_in);

        // if this is not a road, do not include
        if (rec1.CFCC[0] != 'A')  continue;
        ++num_segments;

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.FRADDL, sizeof(rec1.FRADDL));
        segment.StartAddressLeft = atoi(str);

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.TOADDL, sizeof(rec1.TOADDL));
        segment.EndAddressLeft = atoi(str); 

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.FRADDR, sizeof(rec1.FRADDR));
        segment.StartAddressRight = atoi(str);

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.TOADDR, sizeof(rec1.TOADDR));
        segment.EndAddressRight = atoi(str);

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.FRLAT, sizeof(rec1.FRLAT));
        segment.StartPoint.Latitude = atoi(str);

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.FRLONG, sizeof(rec1.FRLONG));
        segment.StartPoint.Longitude = atoi(str);

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.TOLAT, sizeof(rec1.TOLAT));
        segment.EndPoint.Latitude = atoi(str);

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.TOLONG, sizeof(rec1.TOLONG));
        segment.EndPoint.Longitude = atoi(str);

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.CFCC, sizeof(rec1.CFCC));
        if (rec1.CFCC[0] == 'H')  // water body?
            segment.RoadClass = 127;
        else
            segment.RoadClass = atoi(&str[1]);

        memset(str, 0, sizeof(str));
        memcpy(str, rec1.TLID, sizeof(rec1.TLID));
        tlid = atoi(str);

        segment.ShapeIndex = get_shape_index(tlid, chains, fp_chains);
        segment.StreetIndex = get_street_index(rec1.FEDIRP);

        fwrite(&segment, sizeof(segment), 1, fp_segments);

        percent_complete = 100 * i / numRecs;
        if (percent_complete > prev_percent)
        {
            printf(" %.2d%\b\b\b\b", percent_complete);
            fflush(stdout);
            prev_percent = percent_complete;
        }
    }

    printf( "Segments output to file = %d \n", num_segments );
    printf( "-----------------------------------------------\n\n" );

    fclose( fp_in );
}


/**
* This method returns the index in the Shape Points list for the specified 
* TLID. Once a chain (of shape points) is found, it is also appended to 
* chains.dat
*
* Returns the shape index, -1 if there is no chain for the TLID.
*/
int get_shape_index(int tlid, struct _Chains *shapes, FILE *fp_chains)
{
    int i;

    // find the index for the shape points   
    for (i = 0; i < num_chains_in; i++)
    {
        if (shapes[i].tlid == tlid)
        {
            fwrite(&shapes[i].num_points, sizeof(int), 1, fp_chains);
            fwrite(shapes[i].points, sizeof(struct _Coordinates), 
                shapes[i].num_points, fp_chains);
            ++num_chains_out;

            //printf("%d: %d\n", num_chains_out-1, shapes[i].num_points);
            return num_chains_out-1;
        }
    }

    // if we reached here, there is no matching TLID.  i.e. the street is a 
    // straight line.
    return -1;
}


/**
* This method the index in the Address list where the specified street 
* address is located.  If not already present in the list, it is added.
*
* Returns the street index.
*/
int get_street_index(char *name)
{
    int i;

    // check to see if street is already in array
    for (i = street_index_start; i < num_streets; i++)
    {
        if (!memcmp(name, &street[i], sizeof(struct _StreetName)))
            return i;
    }

    // if we reached here, the street does not exist in array, so add it.
    ++num_streets;

    // if not enough memory has been allocated..
    if (num_streets > allocated_mem)
    {
        allocated_mem += 1000;  // allocated in chunks of 1000
        street = (struct _StreetName *) realloc( street, allocated_mem * 
            sizeof(struct _StreetName) );
    }

    // add the new street into array
    memcpy(&street[num_streets-1], name, sizeof(struct _StreetName));

    return num_streets-1;
}

