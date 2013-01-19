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
#include <math.h>
#include "gd.h"
#include "gdfontmb.h"
#include "tmrs.h"


struct _StreetLabel 
{
    int street_index;
    char road_class;
    int x1, x2, y1, y2;
    struct _StreetLabel *next;
};

struct _StreetLabel *head;

int background, minor_street, major_street, highway, black, blue;
int gray, green;
int light_gray, dark_gray;
int streets[16], num_printed;


/**
* Returns the square of the distance between two specified points
*/
int length(int x1, int y1, int x2, int y2)
{
    int a, b;

    a = x2-x1;
    b = y2-y1;

    return(a*a + b*b);
}


/**
* This functions adds a street segment to the linked list.  This list is 
* traversed at the end of the map drawing and the labels then printed.  
*/
void add_street_label(int i, int x1, int y1, int x2, int y2)
{
    struct _StreetLabel *cur_ptr, *prev_ptr, *pNode;
    int temp, d1, d2;

    // swap coordinates if necessary so that label is always printed above 
    // the street for horizontal cases.
    if (x2 < x1)
    {
        temp = x1;
        x1 = x2;
        x2 = temp;

        temp = y1;
        y1 = y2;
        y2 = temp;
    }

    cur_ptr = head;
    while (cur_ptr != NULL)
    {
        if (cur_ptr->street_index == segment[i].StreetIndex)
        {
            // check if we should replace current with new segment
            d1 = length(cur_ptr->x1, cur_ptr->y1, cur_ptr->x2, cur_ptr->y2);
            d2 = length(x1, y1, x2, y2);

            if (d2 > d1)
            {
                cur_ptr->x1 = x1;  cur_ptr->y1 = y1;
                cur_ptr->x2 = x2;  cur_ptr->y2 = y2;
            }
            //printf("replaced street\n");
            return;
        }
        cur_ptr = cur_ptr->next;
    }

    // reached here means we did not encounter this street name before
    pNode = (struct _StreetLabel *)malloc(sizeof(struct _StreetLabel));
    pNode->street_index = segment[i].StreetIndex;
    pNode->road_class = segment[i].RoadClass;
    pNode->x1 = x1; pNode->y1 = y1;
    pNode->x2 = x2; pNode->y2 = y2;
    pNode->next = NULL;

    // handle situation where this is the first node in list
    //printf("Adding street\n");
    if (head == NULL)
    {
        head = pNode;
        return;
    }

    // insert node at the correct point sorted by RoadClass
    cur_ptr = head;
    prev_ptr = cur_ptr;
    while (cur_ptr != NULL)
    {
        if (pNode->road_class < cur_ptr->road_class)
        {
            pNode->next = cur_ptr;
            if (cur_ptr == head)
                head = pNode;
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
* This function draws a line based on the information passed to it.
*
* &c       - a pointer to the coordinates on which to center the map.
* scale    - the scale of the map.  Good range is 10 to 2000.
* road_class - the type of road.  This is used to draw using appropriate 
*              colors and width.
* im       - the image on which to draw.
*/
void draw_line(struct _Coordinates *c, struct _Coordinates *m, struct _Coordinates *n,
               int scale, int i, gdImagePtr im)
{
    int x1, y1, x2, y2, k;

    x1 = im->sx/2 + ((abs(c->Longitude) - abs(m->Longitude)) / scale);
    y1 = im->sy/2 + ((c->Latitude - m->Latitude) / scale);
    x2 = im->sx/2 + ((abs(c->Longitude) - abs(n->Longitude)) /scale);
    y2 = im->sy/2 + ((c->Latitude - n->Latitude) / scale);

    // draw only if line intersects the image window
    if ((CONTAINS(0,im->sx,x1) && CONTAINS(0, im->sy, y1)) || 
        (CONTAINS(0,im->sx,x2) && CONTAINS(0, im->sy, y2)))
    {
        // draw highways as thick red lines
        if (segment[i].RoadClass < 20)
        {
            gdImageSetThickness(im, 5);
            gdImageLine(im, x1, y1, x2, y2, dark_gray);
            gdImageSetThickness(im, 3);
            gdImageLine(im, x1, y1, x2, y2, highway);
            add_street_label(i,x1,y1,x2,y2);
        }

        // draw major streets appropriately based on scale
        else if (segment[i].RoadClass < 30)
        {
            if (scale < 400)
            {
                gdImageSetThickness(im, 4);
                gdImageLine(im, x1, y1, x2, y2, dark_gray);
                gdImageSetThickness(im, 2);
                gdImageLine(im, x1, y1, x2, y2, major_street);
                add_street_label(i,x1,y1,x2,y2);
            }
            else if (scale < 3000)
            {
                gdImageSetThickness(im, 1);
                gdImageLine(im, x1, y1, x2, y2, dark_gray);
            }
        }

        // draw minor streets (if at all) based on scale
        else if (segment[i].RoadClass < 70)
        {
            if (scale < 30)
            {
                gdImageSetThickness(im, 4);
                gdImageLine(im, x1, y1, x2, y2, dark_gray);
                gdImageSetThickness(im, 2);
                gdImageLine(im, x1, y1, x2, y2, light_gray);
                add_street_label(i,x1,y1,x2,y2);
            }
            else if (scale < 400)
            {
                gdImageSetThickness(im, 1);
                gdImageLine(im, x1, y1, x2, y2, dark_gray);
                //add_street_label(i,x1,y1,x2,y2);
            }
        }

        // this is a water boundary
        else if (segment[i].RoadClass == 127)
        {
            gdImageSetThickness(im, 1);
            gdImageLine(im, x1, y1, x2, y2, blue);
        }
    }
}

/**
* This function draws a filled polygon based on the information passed to it.
*
* &c       - a pointer to the coordinates on which to center the map.
* scale    - the scale of the map.  Good range is 10 to 2000.
* &p       - pointer to a polygon structure.
* im       - the image on which to draw.
*/
void draw_polygon(struct _Coordinates *c, struct _Polygon *p, int scale, 
                  gdImagePtr im)
{
    int i, x, y, color;
    struct _Coordinates *m;
    gdPoint *gp;

    gp = (gdPoint *)malloc(p->num_points * sizeof(gdPoint));

    for (i = 0; i < p->num_points; i++)
    {
        m = &p->point[i];
        x = im->sx/2 + ((abs(c->Longitude) - abs(m->Longitude)) / scale);
        y = im->sy/2 + ((c->Latitude - m->Latitude) / scale);

        gp[i].x = x;
        gp[i].y = y;
    }

    // select appropriate color
    if (p->type == POLYGON_WATER)  
        color = blue;
    else if (p->type == POLYGON_AIRPORT)
        color = gray;
    else 
        color = green;

    gdImageFilledPolygon(im, gp, p->num_points, color);
    free(gp);
}


/**
* This function draw a specified road segment.  If it happens to contain 
* shape points, then each sub-segment will be drawn separately.
*/
void draw_segment(int i, struct _Coordinates *c, int scale, gdImagePtr im)
{
    int num_points, shapeIndex, j;
    struct _Coordinates *point;  

    shapeIndex = segment[i].ShapeIndex;
    if (shapeIndex < 0)
    {
        draw_line(c, &segment[i].StartPoint, &segment[i].EndPoint, 
            scale, i, im);
    }
    else
    {
        num_points = shape[shapeIndex].num_points;
        point = shape[shapeIndex].point;

        draw_line(c, &segment[i].StartPoint, &point[0], scale, i, im);

        for (j = 0; j < shape[shapeIndex].num_points-1; j++)
            draw_line(c,&point[j], &point[j+1], scale, i, im);

        draw_line(c, &point[num_points-1], &segment[i].EndPoint, scale, i, im);
    }
}


/**
* This function outputs the image in a raw format to the specified sink. The 
* sink is generally stdio or a socket.  The size of the output is 
*
*    total_bytes = width * height * 4 bytes
*
* i.e. 4 bytes / pixel  ( R G B A )
*/
void image_raw_to_sink(gdImagePtr im, gdSinkPtr pSink)
{
    int i, nbytes, nwritten, res;

    nbytes = im->sx * sizeof(int);  // true color = 4 bytes/pixel

    for (i = 0; i < im->sy; i++)
    {
        nwritten = 0;
        while (nwritten < nbytes) {
            res = pSink->sink(pSink->context, &im->tpixels[i][nwritten], 
                (im->sx - nwritten) * sizeof(int));
            if (res < 0) return;  //error occurred
            nwritten += res;
        }
    }
}


/**
* Starting point of a map-drawing operation.  
*
* format - the output file type: RAW or PNG
* width - the width of the output image
* height - the height of the output image
* &c  - the coordinates on which to center the map
* scale - the scale of the map.  try ranges of 10 to 3000
*/
int draw_map(char *format, int width, int height, struct _Coordinates *c, 
             int scale, gdSink *pSink) 
{
    gdImagePtr im;
    FILE *pngout;
    int i, x1, x2, y1, y2, label_count;
    int x_pos, y_pos;
    struct _StreetLabel *prev_ptr;
    int brect[8];
    char str[64];
    char *f = "./arial.ttf";
    double angle;
    float font_size = 10.0;

    FILE *fp;
    int j;


    head = NULL;
    num_printed = 0;

    im = gdImageCreateTrueColor(width, height);

    background = gdTrueColor(254, 247, 230);
    light_gray = gdTrueColor(238, 238, 238);
    dark_gray = gdTrueColor(164, 164, 164);
    major_street = gdTrueColor(253, 215, 101);
    highway = gdTrueColor(224, 96, 0);
    black = gdTrueColor(0, 0, 0);
    blue = gdTrueColor(166, 202, 240);
    green = gdTrueColor(156, 211, 156);
    gray = gdTrueColor(192,192,192);

    gdImageFilledRectangle(im, 0, 0, width, height, background);

    /* draw the filled water polygons first */
    for (i = 0; i < numPolygons; i++)
        draw_polygon(c, &polygon[i], scale, im);

    /* draw small streets and water first */
    for (i = 0; i < numRecs; i++)
    {
        if (segment[i].RoadClass < 30) continue;
        draw_segment(i, c, scale,im );
    }

    /* draw major streets */
    for (i = 0; i < numRecs; i++)
    {
        if (segment[i].RoadClass < 20 || segment[i].RoadClass > 29) continue;
        draw_segment(i, c, scale,im );   
    } 

    /* draw highways */
    for (i = 0; i < numRecs; i++)
    {
        if (segment[i].RoadClass >= 20) continue;
        draw_segment(i, c, scale,im );       
    }

    /* now draw the labels and destroy linked list at the same time */
    // destroy the linked list
    label_count = 0;  // don't print more than 5 labels
    prev_ptr = head;
    while (head != NULL)
    {
        head = head->next;
        ++label_count;

        if (label_count < 5)
        {
            x1 = prev_ptr->x1; 
            y1 = prev_ptr->y1;
            x2 = prev_ptr->x2; 
            y2 = prev_ptr->y2;
            angle = atan2(y1-y2, x2-x1);


            if (prev_ptr->road_class < 30)
                font_size = 12.0;
            else
                font_size = 10.0;

            format_street_name(str, prev_ptr->street_index);
            //printf("%s - %f \n", str, angle);

            // figure out the x and y position at which to draw the label
            gdImageStringFT(NULL,&brect[0],0,f,font_size,angle, 0, 0, str);

            x_pos = x1 + ((x2 - x1) - (brect[2] - brect[0]))/2;
            if (y2 > y1)
                y_pos = y1 + ((y2 - y1) - (brect[3] - brect[1]))/2;
            else
                y_pos = y2 + ((y1 - y2) - (brect[3] - brect[1]))/2;

            if (angle > 0.0 && angle < 1.6)
                x_pos -= 4;
            else
                x_pos += 4;

            gdImageStringFT(im,&brect[0],black,f,font_size,angle,x_pos, y_pos-4, str);
        }

        free(prev_ptr);
        prev_ptr = head;
        //printf("deleted street %d %d\n", x1, y1);
    }

    if (strcmp(format, "PNG") == 0)
        gdImagePngToSink(im, pSink);
    else {
        image_raw_to_sink(im, pSink);
    }

    gdImageDestroy(im);
}
