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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "gd.h"
#include "tmrs.h"

// function prototypes
void print_address(char *name, char *type);
void load_segments_file(char *);
void load_shapes_file(char *);
void load_names_file(char *);
void load_polygons_file(char *);
int find_closest_highway(struct _Coordinates *m);



static int stdioSink(void *context, char *buffer, int len)
{
    return fwrite(buffer, 1, len, (FILE *) context);
}


int main(int argc, char **argv)
{
    int i, source, destination, waypoint1, waypoint2, optchar;
    int run_server = 0;
    float d;
    char *data_dir = "./";   // default directory
    char str[64], *street = NULL, *map_string = NULL;
    char segments_filename[256], names_filename[256], shapes_filename[256];
    char polygons_filename[256];
    gdSink mySink;
    FILE *fp;

    /* 
    * Read in the command line parameters.  The following parameters are allowed
    *  -d <path_to_data_files>
    *  -s <run as server>
    *  -m <comma_separated_map_string>
    *  -a <comma_separated_street_address>
    */
    while ((optchar = getopt (argc, argv, "d:a:m:s")) != -1)
    {
        switch (optchar)
        {
        case 'd':
            data_dir = (char *) strdup (optarg);
            break;

        case 's':
            run_server = 1;
            break;

        case 'a':
            street = (char *) strdup (optarg);
            break;

        case 'm':
            map_string = (char *) strdup (optarg);
            break;

        default:
        case '?':
            printf ("Usage: %s [-d datadir] [-s] [-a address_string] [-m map_string]\n\n", argv[0]);
            return EXIT_FAILURE;
        }
    }

    // initialize pointers used in A* Search
    open_list_head = NULL;
    closed_list_head = NULL;

    // makes sure these files exist, otherwise you get a segmentation fault!
    sprintf(segments_filename, "%s/%s", data_dir, "segments.dat");
    sprintf(names_filename, "%s/%s", data_dir, "names.dat");
    sprintf(shapes_filename, "%s/%s", data_dir, "chains.dat");
    sprintf(polygons_filename, "%s/%s", data_dir, "polygons.dat");
    load_segments_file(segments_filename);
    load_names_file(names_filename);
    load_shapes_file(shapes_filename);
    load_polygons_file(polygons_filename);

    mySink.context = (void *) stdout;
    mySink.sink = stdioSink;

    if (run_server == 1)        // run as server? (-s option on command line)
        server_start();
    else if (street != NULL)    // address search request?
        handle_find_address(street, &mySink);
    else if (map_string != NULL) 
        handle_draw_map(map_string, &mySink);   


    //destination = find_address(8102, "", "Gulf",  "Dr", "");
    //destination = find_address(5024, "W", "Nassau",  "St", "");
    //destination = find_address(0, "", "Morris Bridge",  "", "");

    /*d = get_distance(&segment[source].StartPoint, &segment[destination].StartPoint);
    printf("Distance = %f miles\n\n", d);

    if (d > 10.0)
    {
    printf("Distance is too great, split A*\n");

    waypoint1 = find_closest_highway(&segment[source].StartPoint);
    format_street_name(str, segment[waypoint1].StreetIndex);
    printf("Closest highway is %s\n", str);
    find_shortest_path(source,waypoint1,0);

    waypoint2 = find_closest_highway(&segment[destination].StartPoint);
    format_street_name(str, segment[waypoint2].StreetIndex);
    printf("Closest highway is %s\n", str);
    find_shortest_path(destination,waypoint2,0);

    find_shortest_path(waypoint1,waypoint2,1);
    } else {
    find_shortest_path(source,destination,0);
    } */


    /*fp = fopen("test.png", "wb");
    mySink.context = (void *) fp;
    mySink.sink = stdioSink;
    draw_map(800, 600, &segment[destination].StartPoint,20, &mySink);
    fclose(fp);*/

    free(segment);
    free(street);
    free(shape);
    free(polygon);

    return EXIT_SUCCESS;
}


/**
* This function loads the specified file into memory pointed to by segment
*/
void load_segments_file(char *filename)
{
    int length;
    FILE *fp;

    fp = fopen( filename, "r" );
    if (fp == NULL)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    //goto end of file
    fseek( fp, 0, SEEK_END );

    //get length of file and number of records
    length = ftell( fp );
    numRecs = length/sizeof(struct _RoadSegment);

    //go back to beginning of file
    fseek( fp, 0, SEEK_SET );

    // allocate memory to hold all the road segments
    segment = (struct _RoadSegment *) malloc(numRecs * sizeof(struct _RoadSegment));

    // read all records from file
    fread(segment, numRecs*sizeof(struct _RoadSegment), 1, fp);

    // we are done with the file
    fclose( fp );
}


/**
* This function loads the specified file into memory pointed to by street
*/
void load_names_file(char *filename)
{
    int length;
    FILE *fp;

    fp = fopen( filename, "r" );
    if (fp == NULL)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    //goto end of file
    fseek( fp, 0, SEEK_END );

    //get length of file and number of records
    length = ftell( fp );
    numStreets = length/sizeof(struct _StreetName);

    //go back to beginning of file
    fseek( fp, 0, SEEK_SET );

    // allocate memory to hold all the road segments
    street = (struct _StreetName *) malloc(numStreets * sizeof(struct _StreetName));

    // read all records from file
    fread(street, numStreets*sizeof(struct _StreetName), 1, fp);

    // we are done with the file
    fclose( fp );
}


/**
* This function loads the specified file into memory pointed to by shape
*/
void load_shapes_file(char *filename)
{   
    FILE *fp;
    int i;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    fread(&numShapes, sizeof(int), 1, fp);
    shape = (struct _ShapePoints *) malloc(numShapes * sizeof(struct _ShapePoints));

    //printf("Number of chains = %d\n", numShapes);

    for (i = 0; i < numShapes; i++)
    {
        fread(&shape[i].num_points, sizeof(int), 1, fp);
        shape[i].point = (struct _Coordinates *)malloc(shape[i].num_points * sizeof(struct _Coordinates));
        fread(shape[i].point, sizeof(struct _Coordinates), shape[i].num_points, fp);

        //printf("%d: Number points = %d\n", i, shape[i].num_points);
    }

    fclose(fp);
}


/**
* This function loads the specified file into memory pointed to by polygon
*/
void load_polygons_file(char *filename)
{
    FILE *fp;
    int i;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        // Polygons data file is not absolutely necessary
        //perror(filename);
        return;
    }

    fread(&numPolygons, sizeof(int), 1, fp);
    polygon = (struct _Polygon *) malloc(numPolygons * sizeof(struct _Polygon));

    //printf("Num polygons = %d\n", numPolygons);

    for (i = 0; i < numPolygons; i++)
    {
        fread(&polygon[i].type, sizeof(char), 1, fp);
        fread(polygon[i].name, sizeof(polygon[i].name), 1, fp);
        fread(&polygon[i].num_points, sizeof(int), 1, fp);
        //printf("%d: %d\n", i, polygon[i].num_points);
        polygon[i].point = (struct _Coordinates *)malloc(polygon[i].num_points * sizeof(struct _Coordinates));
        fread(polygon[i].point, sizeof(struct _Coordinates), polygon[i].num_points, fp);
    }

    fclose(fp);
}


/**
* Finds the coordinates of the requested address and sends the output to the 
* supplied sink (stdout or socket).  The format of the address string is the 
* following:  
*
*      "<number>,<prefix>,<name>,<type>,<suffix>"
*  eg: "4202,E,Fowler,Ave,*"  (note that empty strings should be passed as
*                               asterisk)
*
* All matching addresses are returned in the following format:
*      <index>:<segment_index>:<address in request format>:Latitude,Longitude
*/
void handle_find_address(char *address, gdSink *pSink)
{
    int i, j, street_number, match_count = 0;
    char *prefix, *name, *type, *suffix, *empty = "";
    char str[256], str2[256];
    const char *delimiters = ","; 

    // extract the fields out of the request message
    street_number = atoi(strtok (address, delimiters));
    prefix = strtok(NULL, delimiters);
    name = strtok(NULL, delimiters);
    type = strtok(NULL, delimiters);
    suffix = strtok(NULL, delimiters);

    // check for invalid format
    if (!prefix || !name || !type || !suffix) 
    {
        sprintf(str, "E:Invalid address format.\n", address);
        pSink->sink(pSink->context, str, strlen(str));
        return;
    }

    // Handle wildcards
    if (prefix[0] == '*') prefix = empty;
    if (name[0] == '*') name = empty;
    if (type[0] == '*') type = empty;
    if (suffix[0] == '*') suffix = empty;

    // first find the index of the street name
    for (i = 0; i < numStreets; i++)
    {
        // if too many matches, then exit search
        if (match_count > 10) 
        {
            sprintf(str, "E:Too many matches.  Narrow search.\n", street_number, 
                prefix, name, type, suffix);
            pSink->sink(pSink->context, str, strlen(str));
            return;
        }

        // check if the text part of street address matches
        if (!strncmp(street[i].prefix, prefix, strlen(prefix)) &&
            !strncmp(street[i].name, name, strlen(name)) && 
            !strncmp(street[i].type, type, strlen(type)) &&
            !strncmp(street[i].suffix, suffix, strlen(suffix)))
        {
            // if a street number was not provided, no point in interating through segments
            if (street_number == 0)
            {
                format_street_name(str, i);
                sprintf(str2, "A:%d:x %s:%d,%d\n", j, str, segment[j].StartPoint.Latitude, 
                    segment[j].StartPoint.Longitude);
                pSink->sink(pSink->context, str2, strlen(str2));
                ++match_count;
                continue;
            }

            // now look for the street number in the segments array
            for (j = 0; j < numRecs; j++)
            {
                if (segment[j].StreetIndex == i)
                {
                    if (CONTAINS(segment[j].StartAddressLeft, segment[j].EndAddressLeft, street_number) ||
                        CONTAINS(segment[j].StartAddressRight, segment[j].EndAddressRight, street_number))
                    {
                        format_street_name(str, i);
                        sprintf(str2, "A:%d:%d %s:%d,%d\n", j, street_number, str, 
                            segment[j].StartPoint.Latitude, segment[j].StartPoint.Longitude);
                        pSink->sink(pSink->context, str2, strlen(str2));
                        ++match_count;
                    }
                }
            }
        }
    }

    // if not found, return
    if (match_count == 0) 
    {
        sprintf(str, "E:Address not found.\n", street_number, prefix, name, type, suffix);
        pSink->sink(pSink->context, str, strlen(str));
    }
}


/**
* Draws a map and sends the output image to the provided sink (stdout or 
* socket).  The format of the request string is as follows:
*
*      "<format>,<img_width>,<img_height>,<scale>,<lat>,<long>"
*
*      eg - "PNG,640,480,100,27954297,-82828517"
*/
void handle_draw_map(char *str, gdSink *pSink)
{
    gdSink mySink;
    struct _Coordinates p;
    int width, height, scale;
    char format[16];
    const char delimiters[] = ",";

    //printf("Inside handle_map\n");

    // process request parameters
    strncpy(format, strtok (str, delimiters), sizeof(format)-1); 
    width = atoi(strtok (NULL, delimiters));
    height = atoi(strtok (NULL, delimiters));
    scale = atoi(strtok (NULL, delimiters));
    p.Latitude = atoi(strtok (NULL, delimiters));
    p.Longitude = atoi(strtok (NULL, delimiters));

    //printf("server: request (format:%s  w:%d  h:%d  scale:%d  lat:%d  long:%d \n", 
    //     format, width, height, scale, p.Latitude, p.Longitude);

    // check the passed parameters for errors
    if (scale < 1) {
        printf("E:Invalid request.\n");
        return;
    }

    // call draw_map with appropriate parameters
    draw_map(format, width, height, &p, scale, pSink);
}


/**
* Returns the closest highway to the given point.  The index of the highway 
* segment is what is returned if found.  Otherwise -1 is returned.
*/
int find_closest_highway(struct _Coordinates *m)
{
    float d, min_distance;
    int segment_index;
    int i;

    min_distance = 99999.0;
    segment_index = -1;

    for (i = 0; i < numRecs; i++)
    {
        if (segment[i].RoadClass > 19)  continue;

        d = get_manhattan_distance(&segment[i].StartPoint, m);
        if (d < min_distance)
        {
            min_distance = d;
            segment_index = i;
        }

        d = get_manhattan_distance(&segment[i].EndPoint, m);
        if (d < min_distance)
        {
            min_distance = d;
            segment_index = i;
        }
    }

    printf("Shortest distance = %f miles\n", min_distance);

    return segment_index;
}

