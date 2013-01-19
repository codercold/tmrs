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
 * This function reads in a xxxxx.RTI file and retrieves the Polygon to Chain 
 * linkage records.  The memory allocated to variable poly must be freed by 
 * the caller.
 *
 * Returns the number of records read from file.
 */
int get_polygon_chain_linkages(char *filename, struct _PolygonChainLink **linkage)
{
    FILE *fp_in;
    int i, index, length, numRecs, numWater=0;
    char str[32];
    struct _RecordTypeI rec;
    struct _PolygonChainLink *myLinkage;

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
    numRecs = length / sizeof(struct _RecordTypeI);
    printf("Length of input file \t= %d bytes\nNumber of records \t= %d\n", 
        length, numRecs);

    // go back to beginning of file
    fseek( fp_in, 0, SEEK_SET );

    // populate array of struct _PolygonID on the second iteration
    index = 0;
    *linkage = (struct _PolygonChainLink *)malloc(numRecs * sizeof(struct _PolygonChainLink));
    myLinkage = *linkage;

    // go back to beginning of file
    fseek( fp_in, 0, SEEK_SET );

    for (i = 0; i < numRecs; i++)
    {
        fread(&rec, sizeof(struct _RecordTypeI), 1, fp_in);
        
        // copy the left and right CENIDs
        memcpy(myLinkage[i].left_polygon_id.cenid, rec.CENIDL, sizeof(rec.CENIDL));
        memcpy(myLinkage[i].right_polygon_id.cenid, rec.CENIDR, sizeof(rec.CENIDR));

        // convert to int and copy the POLYID
        memset(str, 0, sizeof(str));
        memcpy(str, rec.POLYIDL, sizeof(rec.POLYIDL));
        myLinkage[i].left_polygon_id.polyid = atoi(str);

        memset(str, 0, sizeof(str));
        memcpy(str, rec.POLYIDR, sizeof(rec.POLYIDR));
        myLinkage[i].right_polygon_id.polyid = atoi(str);

        // convert to int and copy TLID
        memset(str, 0, sizeof(str));
        memcpy(str, rec.TLID, sizeof(rec.TLID));
        myLinkage[i].tlid = atoi(str);
    }

    fclose(fp_in);

    return numRecs;
}

