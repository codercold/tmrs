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


/**
* The purpose of this function is to take an .RT2 file and write it to a
* file in a compressed form.
*
* Returns 0 if successful.
*/
int compress_rt2(char *filename, char *output_file)
{
    FILE *fp_in, *fp_out;
    char str[32];
    int i, j, length, numRecs, num_points, num_chains;
    int tlid, prev_tlid;
    int longitude, latitude;
    struct _RecordType2 rec2;
    fpos_t pos1, pos2;  

    printf("\nCompressing: %s --> %s\n", filename, output_file);
    printf( "---------------------------------------------\n" );

    /* open the source RT2 file */
    fp_in = fopen( filename, "r" ) ;
    if (fp_in == NULL)
    {
        perror(filename);
        return 1;
    }

    /* open the output file for writing */
    fp_out = fopen( output_file, "w" );
    if (fp_out == NULL)
    {
        perror(filename);
        return 2;
    }

    //goto end of file
    fseek( fp_in, 0, SEEK_END );

    //get length of file
    length = ftell( fp_in );
    numRecs = length / sizeof(struct _RecordType2);
    printf("Length of input file \t= %d bytes\nNumber of records \t= %d\n", 
        length, numRecs);

    // go back to beginning of file
    fseek( fp_in, 0, SEEK_SET );

    prev_tlid = -1;
    num_chains = 0;

    // set the main number of chains count (zero right now)
    fwrite(&num_chains, sizeof(int), 1, fp_out);

    for (i = 0; i < numRecs; i++)
    {
        fread(&rec2, sizeof(struct _RecordType2), 1, fp_in);
        tlid = atoi(rec2.TLID);

        /* 
        * if this is the start of a new chain, then write TLID and num_points 
        * to file. Note that num_points is zero at this time.  After we have 
        * counted the number of points in the chain, we come back to this point
        * and update the count.
        */
        if (tlid != prev_tlid)
        {
            // we must fist go back to update the count that was written right 
            // after the previous TLID.
            if (num_chains > 0)
            {
                fgetpos(fp_out, &pos2);
                fsetpos(fp_out, &pos1);
                fwrite(&num_points, sizeof(int), 1, fp_out);
                fsetpos(fp_out, &pos2);
            }

            // start a new line of shape points
            num_points = 0;
            ++num_chains;
            fwrite(&tlid, sizeof(int), 1, fp_out);
            fgetpos(fp_out, &pos1);  // save the position for count 
            fwrite(&num_points, sizeof(num_points), 1, fp_out); 
        }

        /* 
        * go through the 10 coordinates in this record and write each ones to 
        * file if it is not (0,0).  Also update the num_points variable.
        */
        for (j = 0; j < 10; j++)
        {
            memset(str, 0, sizeof(str));
            memcpy(str, rec2.point[j].LONG, sizeof(rec2.point[j].LONG));
            longitude = atoi(str);

            memset(str, 0, sizeof(str));
            memcpy(str, rec2.point[j].LAT, sizeof(rec2.point[j].LAT));
            latitude = atoi(str);

            if (longitude == 0.0 && latitude == 0.0)  // no more points
            {
                break;
            } else
            {
                ++num_points;
                fwrite(&longitude, sizeof(int), 1, fp_out);
                fwrite(&latitude, sizeof(int), 1, fp_out);
            }
        }

        prev_tlid = tlid;
    }

    // need to update the last count
    fsetpos(fp_out, &pos1);
    fwrite(&num_points, sizeof(int), 1, fp_out);

    printf( "Length of output file \t= %d bytes\n", ftell(fp_out) );
    printf( "Unique Chains   \t= %d\n", num_chains );
    printf( "---------------------------------------------\n" );

    // update the main 'number of chains' count
    fseek( fp_out, 0, SEEK_SET );
    fwrite(&num_chains, sizeof(int), 1, fp_out);

    fclose(fp_in);
    fclose(fp_out);

    return 0;
}


/**
* Reads the compressed RT2 file (prepared by compress_rt2()) into and array 
* of struct _Chains.  
*
* Returns pointer to array of struct _Chains if successful, NULL otherwise.
* Also sets the value of variable num_chains_in.
*/
struct _Chains *load_compressed_rt2(char *filename)
{
    FILE *fp;
    struct _Chains *shapes;
    int i, tlid;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror(filename);
        return NULL;
    }

    fread(&num_chains_in, sizeof(int), 1, fp);
    shapes = (struct _Chains *) malloc(num_chains_in * sizeof(struct _Chains));

    for (i = 0; i < num_chains_in; i++)
    {
        fread(&shapes[i].tlid, sizeof(int), 1, fp);
        fread(&shapes[i].num_points, sizeof(int), 1, fp);
        shapes[i].points = (struct _Coordinates *)malloc(shapes[i].num_points*sizeof(struct _Coordinates));
        fread(shapes[i].points, sizeof(struct _Coordinates), shapes[i].num_points, fp);
    }

    fclose(fp);

    return shapes;
}


/* free up the memory used by the chains data structure */
void delete_chain_list(struct _Chains *chains)
{
    int i;

    for (i = 0; i < num_chains_in; i++)
        free(chains[i].points);

    free(chains);
    num_chains_in = 0;  // reset the counter
}
