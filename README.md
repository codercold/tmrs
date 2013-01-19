Synopsis
---------
Tiger Mapping and Routing Server (TMRS) is being written in order to facilitate the creation of open source GPS navigation software.  Its goal is to simplify street level routing and map drawing functions essential for developing user-friendly interfaces. The data used in this software is available freely from U.S. Census and is called 'Tiger'. In addition to Tiger data, other sources of navigation data in the form of Shapefiles will also be usable.  Currently, support for ESRI and Navteq dat is planned.  

TMRS will be written in C with no platform-specific dependencies.  It should work on most operating systems even though the development will be done in Linux.  Its design will be dictated by the resource constraints of embedded systems.  It strives to achieve low storage and memory requirements.

There are two services that will be provided by TMRS - 

1.  Street Routing - in this function, the client provides a start and a destination address.  The server computes the best path between the two points and returns driving directions.

2.  Map Server - given a GPS coordinate and area in square miles, the server will return a bitmap that can be used by other applications for a visual display of the area.

In addition to above, a utility program for compressing Tiger Map data will also be created.  


Installation
-------------

TRMS makes use of the GD library available at http://www.boutell.com/gd.  Make sure that you have version 2.0.26 or higher and that it is compiled with PNG and FreeType support.


Here is an output from ldd:

        [sumit@europa tmrs]$ ldd tmrs
                libm.so.6 => /lib/libm.so.6 (0x4001c000)
                libgd.so.2 => /usr/local/lib/libgd.so.2 (0x4003d000)
                libpng12.so.0 => /usr/lib/libpng12.so.0 (0x40071000)
                libc.so.6 => /lib/libc.so.6 (0x40094000)
                libfreetype.so.6 => /usr/lib/libfreetype.so.6 (0x401b2000)
                libz.so.1 => /usr/lib/libz.so.1 (0x401fb000)
                /lib/ld-linux.so.2 => /lib/ld-linux.so.2 (0x40000000)   

If you have the above, it is a simple matter of downloading the source code and typing 'make' to compile. 


Running the software
--------------------

Assume that tmrs is installed at the following location: /tmrs
Assume that TIGER data files for your county are at: /tmrs/data/TIGER/

1.  cd /tmrs/src
2.  make
3.  cd /tmrs/src/TIGER
4.  make

At this point, you should have two executables: /tmrs/src/tmrs and /tmrs/src/TIGER/convert.  Convert is used for processing TIGER data files into a form which tmrs can understand.  Execute the following:

        /tmrs/src/TIGER/convert -d /tmrs/data/TIGER

This will create a few files in the data directory.  Now you are ready for drawing maps.  The first step is to locate your address:

        /tmrs/src/tmrs -d /tmrs/data/TIGER -a 4202,E,Fowler,Ave,*
        /tmrs/src/tmrs -d /tmrs/src/TIGER -m PNG,640,480,100,28054495,-82416015 > map.png

View your new map.png.


Troubleshooting
---------------

1) Streets are not labelled.
    - Make sure your GD library has support for TrueType compiled in.
    - Make sure arial.ttf is present in the same directory as tmrs executable.


Sources of map data
-------------------

TIGER 2002:  This is freely available from the U.S. Census Bureau website.  This data is only for the United States.  Data sets are divided by counties.  Each county consists of various files out of which we only need the files with extension .RT1 and .RT2.  These files contain line data (streets).  As of this writing, the website for download was:  http://www.census.gov/geo/www/tiger/tiger2002/tgr2002.html

ESRI Shapefiles:  ESRI makes Tiger 2000 data available in Shapefile format.  These are also free.  You can select which layers you want to download.  The ones that TMRS uses are Line Features (streets) and Land Polygons (actually includes water).  Get your files at http://www.esri.com/data/download/census2000_tigerline/index.html

Navteq:  Navteq data is considered to be the most accurate of the bunch.  It is not free, however.  You may have to contact the company to find out the pricing. Sample data can be downloaded at http://www.adci.com

GDT: ???.


