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


// line constants (1 byte)
#define LINE_HIGHWAY            0x01
#define LINE_ROAD_PRIMARY       0x02
#define LINE_ROAD_SECONDARY     0x03
#define LINE_ROAD_NEIGHBORHOOD  0x04
#define LINE_ROAD_TRAIL         0x05

// polygon constants (1 byte)
#define POLYGON_WATER           0x01
#define POLYGON_AIRPORT         0x02
#define POLYGON_PARK            0x03
#define POLYGON_EDUCATION       0x04


// struct to hold longitude and latitude of a point
struct _Coordinates 
{
    int Longitude;
    int Latitude;
};

// a road segment, as stored in the data file
struct _RoadSegment
{
    //int tlid;
    int StartAddressLeft;
    int EndAddressLeft;
    int StartAddressRight;
    int EndAddressRight;
    int StreetIndex;
    int ShapeIndex;
    struct _Coordinates StartPoint;
    struct _Coordinates EndPoint;
    char RoadClass;  // the two numbers after 'A'
};

// list of shape points for a given road segment
struct _ShapePoints
{
    int num_points;
    struct _Coordinates *point;
};

// street name info as stored in data file
struct _StreetName
{
    char prefix[2];
    char name[30];
    char type[4];
    char suffix[2];
};

// each polygon stored in polygons.dat
struct _Polygon
{
    char type;
    char name[30];
    int num_points;
    struct _Coordinates *point; 
};

