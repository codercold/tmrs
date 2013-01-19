CFLAGS=
LIBS=-O -Wall
OBJS=convert.o dbfopen.o shpopen.o

all: ${OBJS}
	gcc ${LIBS} ${OBJS} -o convert 

convert.o:  convert.c shapefil.h
	gcc $(CFLAGS) -c convert.c -o convert.o 

shpopen.o:  shpopen.c shapefil.h
	gcc $(CFLAGS) -c shpopen.c

dbfopen.o:  dbfopen.c shapefil.h
	gcc $(CFLAGS) -c dbfopen.c

clean:  
	rm -f *.o convert

