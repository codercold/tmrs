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
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include "../tmrs_structs.h"
#include "tmrs_extract.h"


/**
* This is the entry point for the program.  The role of this utility is to 
* extract data from TIGER RT1 and RT2 files and compress them suitable for 
* computer use in an embedded environment.  When run from a directory, all 
* RT1 and RT2 files are processed and three new files are output - 
*
* segments.dat - this file essentially holds information from RT1 files such 
*         as endpoints, address ranges and road class.  In addition, it 
*         contains to two indices - name and shape points.
*
* names.dat - this file stores unique street names.  Each street name uses 
*         30 bytes.  Many segments will generally have the same street name 
*         making it worthwhile to create this file and hold just the index 
*         in the segments.dat file (normalization of tables, in a way).
*
* chains.dat - street segments that are not straight lines contains shape 
*         points. Each record in segments.dat may contain an index to an 
*         entry in this file if appropriate. 
*/
int main(int argc, char **argv)
{
    FILE *fp_segments, *fp_chains, *fp_names;
    DIR * dirp;
    struct dirent * dp;
    struct _Chains *chains;
    int len, num_poly, num_link, i, optchar;
    char rt2_filename[256], rt1_filename[256];
    char *data_dir = ".";
    
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

    // open the current directory
    dirp = opendir(data_dir);
    if (dirp == NULL)
    {
        perror(data_dir);
        exit(EXIT_FAILURE);
    }

    // initialize counters
    num_segments = 0;
    num_streets = 0;
    allocated_mem = 0;
    num_chains_in = 0;
    num_chains_out = 0;
    street_index_start = 0;

    // open the output file for writing
    fp_segments= fopen( "segments.dat", "w" );
    fp_chains = fopen( "chains.dat", "w");

    // write the initial chain count to file
    fwrite(&num_chains_out, sizeof(int), 1, fp_chains);

    // find all RT1 files in the directory and process them.
    for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
        len = strlen(dp->d_name);
        if ((len > 4) && !strcmp(&dp->d_name[len-3], "RT1"))
        {
            // set path + filename
            sprintf(rt1_filename, "%s/%s", data_dir, dp->d_name);
            dp->d_name[len-4] = '\0';
            sprintf(rt2_filename, "%s/%s.RT2", data_dir, dp->d_name);
            
            // first process RT2 file and then RT1
            compress_rt2(rt2_filename, "temp.dat");
            chains = load_compressed_rt2("temp.dat");
            process_rt1(rt1_filename, chains, fp_segments, fp_chains);
            delete_chain_list(chains);

            //set the start index of the next pair (new county) of RT1/RT2
            street_index_start = num_streets;
        }
    }

    closedir (dirp);

    // print the statistics
    printf( "Summary\n------------------------------------------------\n");
    printf( "Number of segments   \t= %d, \t%d kB\n", num_segments, 
        num_segments*sizeof(struct _RoadSegment)/1024);
    printf( "Number of shape entries = %d,\t%d kB\n", num_chains_out, 
        ftell(fp_chains)/1024 );
    printf( "Unique street names \t= %d,\t%d kB\n", num_streets, 
        num_streets*sizeof(struct _StreetName)/1024);
    printf( "------------------------------------------------\n\n");

    // close the segment.dat file
    fclose(fp_segments);

    // write the street names out to file
    fp_names = fopen("names.dat", "w");
    fwrite(street, num_streets, sizeof(struct _StreetName), fp_names);
    fclose(fp_names);
    free(street);

    // update the chain count in the output file before closing
    fseek(fp_chains, 0, SEEK_SET);
    fwrite(&num_chains_out, sizeof(int), 1, fp_chains);
    fclose(fp_chains);

    // delete the temporary shape points file
    unlink("temp.dat");

    return EXIT_SUCCESS;
}



