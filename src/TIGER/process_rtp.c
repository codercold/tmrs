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
#include "tmrs_extract.h"


/**
 * This function reads in a xxxxx.RTP file and retrieves the CENID and POLYID
 * values for polygons that represent water bodies.  The memory allocated to 
 * variable poly must be freed by the caller.
 *
 * Returns the number of water polygons found. -1 if an error occurs.
 */
int get_water_polygons(char *filename, struct _PolygonID **poly)
{
    FILE *fp_in;
    int i, index, length, numRecs, numWater=0;
    char str[32];
    struct _RecordTypeP rec;
    struct _PolygonID *myPoly;

    // open the source RTP file
    fp_in = fopen( filename, "r" ) ;
    if (fp_in == NULL)
    {
        perror(filename);
        return -1;
    }

    //goto end of file
    fseek( fp_in, 0, SEEK_END );

    //get length of file
    length = ftell( fp_in );
    numRecs = length / sizeof(struct _RecordTypeP);
    printf("Length of input file \t= %d bytes\nNumber of records \t= %d\n", 
        length, numRecs);

    // go back to beginning of file
    fseek( fp_in, 0, SEEK_SET );

    // find the number of water polygons in the first iteration
    for (i = 0; i < numRecs; i++)
    {
        fread(&rec, sizeof(struct _RecordTypeP), 1, fp_in);
        if (rec.WATER == '1')
            ++numWater;
    }

    // populate array of struct _PolygonID on the second iteration
    index = 0;
    *poly = (struct _PolygonID *)malloc(numWater * sizeof(struct _PolygonID));
    myPoly = *poly;

    // go back to beginning of file
    fseek( fp_in, 0, SEEK_SET );

    for (i = 0; i < numRecs; i++)
    {
        fread(&rec, sizeof(struct _RecordTypeP), 1, fp_in);
        if (rec.WATER == '1')
        {
            // copy the CENID
            memcpy(myPoly[index].cenid, rec.CENID, sizeof(rec.CENID));

            // convert to int and and copy the POLYID
            memset(str, 0, sizeof(str));
            memcpy(str, rec.POLYID, sizeof(rec.POLYID));
            myPoly[index].polyid = atoi(str);
            
            ++index;
        }
    }

    fclose(fp_in);

    return numWater;
}
