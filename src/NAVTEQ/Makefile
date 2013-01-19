default:	all

all:	shpdump dbfdump 

shpopen.o:	shpopen.c shapefil.h
	$(CC) $(CFLAGS) -c shpopen.c

dbfopen.o:	dbfopen.c shapefil.h
	$(CC) $(CFLAGS) -c dbfopen.c

shpdump:	shpdump.c shpopen.o
	$(CC) $(CFLAGS) shpdump.c shpopen.o $(LINKOPT) -o shpdump

dbfdump:	dbfdump.c dbfopen.o
	$(CC) $(CFLAGS) dbfdump.c dbfopen.o $(LINKOPT) -o dbfdump

clean:  
	rm -f *.o shpdump dbfdump

