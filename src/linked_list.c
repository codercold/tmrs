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
* Adds a node to the 'open list'.  The insertion is done in a way to so that 
* node with the lowest F value is always at the head of the linked list. This
* way the list stays sorted.
*/
void open_list_add(struct _GraphNode *node)
{
    struct _ListNode *pNode, *cur_ptr, *prev_ptr;

    pNode = (struct _ListNode *)malloc(sizeof(struct _ListNode));
    pNode->node = node;
    pNode->next = NULL;

    // handle situation where this is the first node in list
    if (open_list_head == NULL)
    {
        open_list_head = pNode;
        return;
    }

    // insert node at the correct point sorted by F-Value
    cur_ptr = open_list_head;
    prev_ptr = cur_ptr;
    while (cur_ptr != NULL)
    {
        if (pNode->node->f_value < cur_ptr->node->f_value)
        {
            pNode->next = cur_ptr;
            if (cur_ptr == open_list_head)
                open_list_head = pNode;
            else
                prev_ptr->next = pNode;
            return;
        }

        prev_ptr = cur_ptr;
        cur_ptr = cur_ptr->next;
    }

    // we reached the end of the list, so this node has largest F-Value
    prev_ptr->next = pNode; 
}


/**
* Add the specified node to the 'closed list'.  These nodes are not considered
* again and may be part of the best path. The specified index is added at the 
* head to spped up things.
*/
void closed_list_add(struct _GraphNode *node)
{
    struct _ListNode *pNode;

    pNode = (struct _ListNode *)malloc(sizeof(struct _ListNode));
    pNode->node = node;
    pNode->next = closed_list_head;
    closed_list_head = pNode;
}


/**
* Removes the specified graph node from the 'open list'.
*/
void open_list_remove(struct _GraphNode *node)
{
    struct _ListNode *cur_ptr, *prev_ptr;

    cur_ptr = open_list_head;
    prev_ptr = cur_ptr;

    // find the node in the linked list and delete it
    while (cur_ptr != NULL)
    {
        if (cur_ptr->node == node)
        {
            if (cur_ptr == open_list_head)
                open_list_head = cur_ptr->next;
            else
                prev_ptr->next = cur_ptr->next;

            free(cur_ptr);
            return;
        }

        prev_ptr = cur_ptr;
        cur_ptr = cur_ptr->next;
    }

    // if we reached here, the node was not found.  This is an error condition.
    printf("open_list_remove: node not found\n");
}


/**
* Tells whether the given node is in the 'open list'.
* Returns pointer to it if present, NULL otherwise.
*/
struct _GraphNode *in_open_list(struct _Coordinates *a)
{
    struct _ListNode *cur_ptr;

    cur_ptr = open_list_head;
    while (cur_ptr != NULL)
    {
        if (same_point(&cur_ptr->node->point, a))
            return cur_ptr->node;

        cur_ptr = cur_ptr->next;
    }

    return NULL;
}


/**
* Tells whether the given node is in the 'closed list'.
* Returns 1 if present, 0 otherwise.
*/
int in_closed_list(struct _Coordinates *a)
{
    struct _ListNode *cur_ptr;

    cur_ptr = closed_list_head;
    while (cur_ptr != NULL)
    {
        //if (same_point(&cur_ptr->node->point, a) == 1);   
        if ((cur_ptr->node->point.Latitude == a->Latitude) &&
            (cur_ptr->node->point.Longitude == a->Longitude))
        {
            return 1;
        }
        cur_ptr = cur_ptr->next;
    }

    return 0;
}


/* Destroys the open list and the corresponding graph nodes */
void open_list_destroy()
{
    struct _ListNode *prev_ptr;

    prev_ptr = open_list_head;
    while (open_list_head != NULL)
    {
        open_list_head = open_list_head->next;
        free(prev_ptr->node);
        free(prev_ptr);
        prev_ptr = open_list_head;
    }
}


/* Destroys the closed list and graph nodes */
void closed_list_destroy()
{
    struct _ListNode *prev_ptr;

    prev_ptr = closed_list_head;
    while (closed_list_head != NULL)
    {
        closed_list_head = closed_list_head->next;
        free(prev_ptr->node);
        free(prev_ptr);
        prev_ptr = closed_list_head;
    }
}

