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
#include "tmrs.h"


/**
* This function returns the speed limit of a road based on its class 
* as specified in the CFCC field of TIGER files.
*/
float get_speed_limit(char road_class)
{
    // Primary Road with limited access/ Interstate Highway - unseparated
    if (road_class >= 11 && road_class <= 14)
        return 70.0;

    // Primary Road with limited access/ Interstate Highway - separated
    else if (road_class >= 15 && road_class <= 18)
        return 80.0;

    // Primary Road without limited access/ US Highway - unseparated
    else if (road_class >= 21 && road_class <= 24)
        return 45.0;

    // Primary Road without limited access / US Highway - separated
    else if (road_class >= 25 && road_class <= 28)
        return 50.0;

    // Secondary and Connecting Roads / State Highways - unseparated
    else if (road_class >= 31 && road_class <= 34)
        return 45.0;

    // Secondary and Connecting Roads / State Highways - separated
    else if (road_class >= 35 && road_class <= 38)
        return 50.0;

    // Local, Rural, Neighborhood / City Street - unseparated
    else if (road_class >= 41 && road_class <= 44)
        return 25.0;

    // Local, Rural, Neighborhood / City Street - separated
    else if (road_class >= 45 && road_class <= 48)
        return 30.0;

    else 
        printf("Unknown road class %d encountered\n", road_class);
}


/**
* This function computes a G Value for the given graph node.  It essentially 
* computes the approximate time that it will take to traverse the segment 
* that is being added to the route.
*
* params:
*    m = the source graph node
*    n = destination graph node
*    
*    'n->belongs_to' is the segment that is being traversed
*/
float get_g_value(struct _GraphNode *m, struct _GraphNode *n)
{
    double d;
    float g;
    int i, num_points, shapeIndex;
    struct _Coordinates *point;

    // check whether segment is a straight line or not
    shapeIndex = segment[n->belongs_to].ShapeIndex;

    // handle straight line
    if (shapeIndex < 0)
        d = get_distance(&m->point, &n->point);

    // handle segment with shape points
    else
    {
        num_points = shape[shapeIndex].num_points;
        point = shape[shapeIndex].point;

        d = get_distance(&segment[n->belongs_to].StartPoint, &point[0]);
        for (i = 0; i < shape[shapeIndex].num_points-1; i++)
        {
            d += get_distance(&point[i], &point[i+1]);   
        }
        d += get_distance(&point[num_points-1], &segment[n->belongs_to].EndPoint);
    }

    // penalize non-interstate segments because there is a chance that 
    // you may get the red light!
    if (segment[n->belongs_to].RoadClass > 19)     // used to be m, CHECK
        d += 0.01;

    // penalize street change a little bit
    if (segment[m->belongs_to].StreetIndex != segment[n->belongs_to].StreetIndex)
        d += 0.08;

    // now divide the distance by an approximate speed based on road class
    g = d / get_speed_limit(segment[n->belongs_to].RoadClass);

    return g;
}


/**
* This function computes a Heuristic Value for the given graph node. It takes 
* the manhattan distance to the destination and divides by a slow speed - 
* 20 miles/hour. This appears to be a decent heuristic, but may require 
* tweaking.
*
*    a = the point being considered
*    b = coordinates of the destination
*/
float get_h_value(struct _Coordinates *a, struct _Coordinates *b)
{
    double d;

    d = get_manhattan_distance(a,b);

    return d/25.0;
}


/*
* This is the kick-off point for the A* Search Algorithm.  It selects an 
* endpoint of the start segment and starts the search from there.  It 
* repeatedly calls process_adjacent_nodes() until there are no nodes in the 
* 'open list' remaining.
*/
void find_shortest_path(int source, int destination, int highwayOnly)
{
    int final,i;
    struct _GraphNode *node1;

    // first seed the 'open list' with the source segment
    node1 = (struct _GraphNode *)malloc(sizeof(struct _GraphNode));
    node1->parent = NULL;
    node1->belongs_to = source;
    node1->SoE = 'a';
    node1->point = segment[source].StartPoint;
    node1->g_value = 0;
    node1->h_value = get_h_value(&segment[source].StartPoint, 
        &segment[destination].StartPoint);
    node1->f_value = node1->g_value + node1->h_value;
    open_list_add(node1);

    // loop while there are elements in the 'open list' or until the 
    // destination is reached.
    while (open_list_head != NULL)
        //for (i = 0; i < 10; i++)
    {
        //   printf("\nConsidering %d%c: ", open_list_head->node->belongs_to,
        //           open_list_head->node->SoE); 
        //   print_segment(open_list_head->node->belongs_to);
        //   printf("\n");
        final = process_adjacent_nodes(open_list_head->node, destination, highwayOnly);
        //   print_closed_list();
        //   print_open_list();

        if (final > 0)
        {
            printf("Reached destination.\n");
            break;
        }
    }

    printf("Done\n");

    // do a little cleanup otherwise subsequent searches will fail.
    open_list_destroy();
    closed_list_destroy();
}


/** 
* Returns true if one of the adjacent streets is the destination. Otherwise -
*
* a. Find segments that intersect at the given node.
* b. If the other end of the found segment is already in closed list, ignore.
* c. If it is already in the 'open list', check to see if the G Value is lower
*    arriving from it.  If so, adjust pointers and values.   
* d. Add adjacent streets to open list (if not already there)
* e. Add current street to the closed list.
*/
int process_adjacent_nodes(struct _GraphNode *node, int dest, int highwayOnly)
{
    int i, found = 0;
    char soe;
    float g;
    char str1[16], str2[5];
    struct _Coordinates other_end;
    struct _GraphNode *new_node, *existing_node;

    for (i = 0; i < numRecs; i++)
    {
        if (highwayOnly && segment[i].RoadClass > 19)  continue;
        if (segment[i].RoadClass > 49)  continue;

        if (same_point(&segment[i].StartPoint, &node->point))
        {
            found = 1;
            other_end = segment[i].EndPoint;
            soe = 'b';
        } 
        else if (same_point(&segment[i].EndPoint, &node->point))
        {
            found = 1;
            other_end = segment[i].StartPoint;
            soe = 'a';
        }

        if (found == 1)
        {
            if (in_closed_list(&other_end))
            {
                continue;
            }

            existing_node = in_open_list(&other_end);
            if (existing_node != NULL)
            {
                // check if G value is lower from current node
                g = existing_node->g_value + get_g_value(existing_node, node);
                if (node->g_value > g)
                {
                    node->g_value = g;
                    node->f_value = g + node->h_value;
                    node->parent = existing_node;
                }
            } 
            else
            {
                new_node = (struct _GraphNode *) malloc(sizeof(struct _GraphNode));
                new_node->point = other_end;
                new_node->parent = node;
                new_node->belongs_to = i;
                new_node->SoE = soe;
                new_node->g_value = node->g_value + get_g_value(node, new_node);
                new_node->h_value = get_h_value(&other_end, &segment[dest].StartPoint);
                new_node->f_value = new_node->g_value + new_node->h_value;
                open_list_add(new_node);
            }

            if (i == dest)
            {
                do
                {
                    print_segment(new_node->belongs_to);
                    printf("\n");
                    new_node = new_node->parent;
                }  while (new_node != NULL);

                return i;  // reached destination, no point in searching further
            }

            found = 0;
        }
    }

    // add the current street to the closed list
    open_list_remove(node);
    closed_list_add(node);

    return -1;  // did not encounter destination
}

