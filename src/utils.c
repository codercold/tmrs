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


#include <math.h>
#include <string.h>
#include "tmrs.h"


/* Gets the approximate distance between two points in miles */
double get_distance(struct _Coordinates *a, struct _Coordinates *b)
{
    double x, y, d;

    x = 0.0000691 * (float)(b->Latitude - a->Latitude);
    y = 0.0000691 * (float)(b->Longitude - a->Longitude) * cos(a->Latitude/57300000.0);

    d = sqrt((x*x) + (y*y));

    return d;
}


/* Gets the approximate distance between two points in miles */
double get_manhattan_distance(struct _Coordinates *a, struct _Coordinates *b)
{
    double x, y;

    x = 0.0000691 * abs(b->Latitude - a->Latitude);
    y = 0.0000691 * abs(b->Longitude - a->Longitude);

    return (x+y);
}


/* Determines whether the two coordinates are the same */
int same_point(struct _Coordinates *a, struct _Coordinates *b)
{
    if ((a->Longitude == b->Longitude) && (a->Latitude == b->Latitude)) {
        return 1;
    }

    return 0;
}


/* prints a segment in human readable form */
void print_segment(int i)
{
    int st_index;
    char str[64];

    st_index = segment[i].StreetIndex;

    format_street_name(str, st_index);   
    /*printf("(A:%d,%d B:%d,%d) \n", 
    segment[i].StartPoint.Longitude, segment[i].StartPoint.Latitude,
    segment[i].EndPoint.Longitude, segment[i].EndPoint.Latitude);
    */
    printf("%s  \t(L:%.4d-%.4d  R:%.4d-%.4d) \t-A%d-", str, 
        segment[i].StartAddressLeft, segment[i].EndAddressLeft,
        segment[i].StartAddressRight, segment[i].EndAddressRight,
        segment[i].RoadClass);  
}


/* formats the street name */
void format_street_name(char *str, int street_index)
{
    int copy_count;

    bzero(str, 64);

    // strip copy the prefix
    for (copy_count = 2; copy_count > 0; --copy_count)
    {
        if (street[street_index].prefix[copy_count-1] != ' ')
        {
            strncpy(str, street[street_index].prefix, copy_count);
            strcat(str, ". ");
            break;
        }
    }

    // strip copy the name
    for (copy_count = 30; copy_count > 0; --copy_count)
    {
        if (street[street_index].name[copy_count-1] != ' ')
        {
            strncat(str, street[street_index].name, copy_count);
            strcat(str, " ");
            break;
        }
    }

    // strip copy the road type
    for (copy_count = 4; copy_count > 0; --copy_count)
    {
        if (street[street_index].type[copy_count-1] != ' ')
        {
            strncat(str, street[street_index].type, copy_count);
            strcat(str, " ");
            break;
        }
    }

    // strip copy the suffix
    for (copy_count = 2; copy_count > 0; --copy_count)
    {
        if (street[street_index].suffix[copy_count-1] != ' ')
        {
            strncat(str, street[street_index].suffix, copy_count);
            break;
        }
    }
}


/* prints the 'open list' in a human readable form */
void print_open_list()
{
    struct _ListNode *prev_ptr;

    prev_ptr = open_list_head;

    printf("Open List: (\n");
    while (prev_ptr != NULL)
    {
        printf("  %d%c: ", prev_ptr->node->belongs_to, prev_ptr->node->SoE);
        print_segment(prev_ptr->node->belongs_to);
        printf("  g=%.2f,h=%.2f,f=%.2f\n", prev_ptr->node->g_value, 
            prev_ptr->node->h_value, prev_ptr->node->f_value);

        prev_ptr = prev_ptr->next;
    }
    printf(" )\n");
}


/* prints the 'closed list' in a human readable form */
void print_closed_list()
{
    struct _ListNode *prev_ptr;

    prev_ptr = closed_list_head;

    printf("Closed List: (\n");
    while (prev_ptr != NULL)
    {
        printf("  %d%c: ", prev_ptr->node->belongs_to, prev_ptr->node->SoE);
        print_segment(prev_ptr->node->belongs_to);
        printf("  g=%.2f,h=%.2f,f=%.2f\n", prev_ptr->node->g_value, 
            prev_ptr->node->h_value, prev_ptr->node->f_value);
        //printf(" %.2f", prev_ptr->node->f_value);
        prev_ptr = prev_ptr->next;
    }
    printf(" )\n");
}



