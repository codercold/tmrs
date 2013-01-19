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

#ifndef _TIGER_H
#define _TIGER_H

/* structs to hold Tiger 2002 map data */

// Record Type 1 - Complete Chain Basic Data Record
struct _RecordType1
{
   char RT;
   char VERSION[4];
   char TLID[10];
   char SIDE1;
   char SOURCE;
   char FEDIRP[2];             
   char FENAME[30];
   char FETYPE[4];             
   char FEDIRS[2]; 
   char CFCC[3];
   char FRADDL[11];
   char TOADDL[11];
   char FRADDR[11];
   char TOADDR[11];
   char FRIADDL;
   char TOIADDL;
   char FRIADDR;
   char TOIADDR;
   char ZIPL[5];
   char ZIPR[5];
   char stuff[74];
   char FRLONG[10];
   char FRLAT[9];
   char TOLONG[10];
   char TOLAT[9];
   char END[2];     //carriage return & newline
} RecordType1;


// Record Type 2 - Complete Chain Shape Coordinates
struct _LONG_LAT
{
   char LONG[10];
   char LAT[9];
};

struct _RecordType2
{
   char RT;
   char VERSION[4];
   char TLID[10];
   char RTSQ[3];
   struct _LONG_LAT point[10];
   char END[2];     //carriage return & newline
} RecordType2;      //210 bytes


// Record Type 7 - Landmark Features
struct _RecordType7
{
    char RT;
    char VERSION[4];
    char FILE[5];
    char LAND[10];
    char SOURCE;
    char CFCC[3];
    char LANAME[30];
    char LALONG[10];
    char LALAT[9];
    char END[3];    // space, carriage return & newline
} RecordType7;      // 76 bytes


// Record Type 8 - Polygons Linked to Area Landmarks
struct _RecordType8
{
    char RT;
    char VERSION[4];
    char FILE[5];
    char CENID[5];
    char POLYID[10];
    char LAND[10];
    char END[3];    // space, carriage return & newline
} RecordType8;      // 38 bytes


// Record Type I - Link Between Complete Chains and Polygons
struct _RecordTypeI
{
    char RT;
    char VERSION[4];
    char FILE[5];
    char TLID[10];
    char TZIDS[10];
    char TZIDE[10];
    char CENIDL[5];
    char POLYIDL[10];
    char CENIDR[5];
    char POLYIDR[10];
    char SOURCE[10];
    char END[49];   // bunch of reserved fields
} RecordTypeI;


// Record Type S - Polygon Geographic Entity Codes
struct _RecordTypeS
{
    char RT;
    char VERSION[4];
    char FILE[5];
    char CENID[5];
    char POLYID[10];
    char END[145];  // stuff we don't care about & newline
} RecordTypeS;


// Record Type P - Polygon Internal Point
struct _RecordTypeP
{
    char RT;
    char VERSION[4];
    char FILE[5];
    char CENID[5];
    char POLYID[10];
    char POLYLONG[10];
    char POLYLAT[9];
    char WATER;
    char END[2];    // newline & carriage return
} RecordTypeP;



#endif
