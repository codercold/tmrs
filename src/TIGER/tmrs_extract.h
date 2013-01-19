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


#ifndef _TMRS_EXTRACT_H
#define _TMRS_EXTRACT_H


// Struct for temporarily holding compressed RT2 Data
struct _Chains
{
    int tlid;
    int num_points;
    struct _Coordinates *points;
};

// Structure for holding POLYID/CENID pairs
struct _PolygonID
{
    char cenid[5];
    int polyid;
};

// Structure for holding RecordType I (polygon-chain linkages)
struct _PolygonChainLink
{
    int tlid;
    struct _PolygonID left_polygon_id;
    struct _PolygonID right_polygon_id;
};

//Functions implemented in process_rt1.c
void process_rt1(char *, struct _Chains *, FILE *, FILE *);
struct _StreetName *street;
int num_segments;
int num_streets, street_index_start, allocated_mem;

// Functions implemented in process_rt2.c
int compress_rt2(char *filename, char *output_file);
struct _Chains *load_compressed_rt2(char *filename);
void delete_chain_list(struct _Chains *chains);
int num_chains_in, num_chains_out;

// Functions implemented in process_rtp.c
int get_water_polygons(char *filename, struct _PolygonID **poly);

// Functions implemented in process_rti.c
int get_polygon_chain_linkages(char *filename, struct _PolygonChainLink **linkage);

#endif

