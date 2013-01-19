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


#ifndef _COMMON_H
#define _COMMON_H

#define CONTAINS(a,b,x)  ( ( x>=a && x<=b ) || ( x>=b && x<=a ) )

#include "gd.h"
#include "tmrs_structs.h"

// struct for storing node info during A* Search.
struct _GraphNode
{
    struct _GraphNode *parent;   // pointing to parent
    struct _Coordinates point;
    float f_value;       // function cost
    float g_value;       // path length to this node
    float h_value;       // heuristic distance to destination
    int belongs_to;      // to which segment does this point belong
    char SoE;            // start or end  
};

// struct for 'open list' and 'closed list' for A* Search Algorithm
struct _ListNode 
{
    struct _GraphNode *node;
    struct _ListNode *next;
};


// global variables
struct _RoadSegment *segment;
struct _StreetName *street;
struct _ShapePoints *shape;
struct _Polygon *polygon;
int numRecs, numStreets, numShapes, numPolygons;;
struct _ListNode *open_list_head;
struct _ListNode *closed_list_head;


//// function prototypes

// functions in tmrs.c
void handle_find_address(char *, gdSink *sink);
void handle_draw_map(char *str, gdSink *pSink);

// functions implemented in linked_list.c
void open_list_add(struct _GraphNode *node);
void closed_list_add(struct _GraphNode *node);
void open_list_remove(struct _GraphNode *node);
struct _GraphNode *in_open_list(struct _Coordinates *a);
int in_closed_list(struct _Coordinates *a);
void open_list_destroy();
void closed_list_destroy();

// functions implemented in a_star.c
void find_shortest_path(int source, int destination, int highwayOnly);
int process_adjacent_nodes(struct _GraphNode *, int, int);

// functions implemented in utils.c
double get_distance(struct _Coordinates *a, struct _Coordinates *b);
double get_manhattan_distance(struct _Coordinates *a, struct _Coordinates *b);
int same_point(struct _Coordinates *a, struct _Coordinates *b);
void print_segment(int i);
void format_street_name(char *str, int street_index);
void print_open_list();
void print_closed_list();

// functions implemented in map.c
int draw_map(char *format, int width, int height, struct _Coordinates *c, 
             int scale, gdSink *sink);

// functions implemented in server.c
void server_start();


#endif
