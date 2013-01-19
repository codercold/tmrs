CFLAGS=
LIBS=-O -Wall 
OBJS=tmrs_extract.o process_rt1.o process_rt2.o

all: ${OBJS} 
	gcc ${LIBS} ${OBJS} -o convert 

tmrs_extract.o: tmrs_extract.c tiger.h tmrs_extract.h
	gcc ${CFLAGS} -c tmrs_extract.c -o tmrs_extract.o   
	
process_rt1.o: process_rt1.c tiger.h tmrs_extract.h
	gcc ${CFLAGS} -c process_rt1.c -o process_rt1.o

process_rt2.o: process_rt2.c tiger.h tmrs_extract.h
	gcc ${CFLAGS} -c process_rt2.c -o process_rt2.o

clean:
	rm -f convert *.o
                                          
