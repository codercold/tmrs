CFLAGS=
LIBS=-O -Wall -lm -lgd -lpng
OBJS=linked_list.o a_star.o tmrs.o utils.o map.o server.o

all: ${OBJS}
	gcc ${LIBS} ${OBJS} -o tmrs 
	
extract: ${OBJS2}
	gcc ${OBJS2} -o tmrs_extract
    
tmrs.o: tmrs.c tmrs.h tmrs_structs.h
	gcc ${CFLAGS} -c tmrs.c -o tmrs.o

a_star.o: a_star.c tmrs.h tmrs_structs.h
	gcc ${CFLAGS} -c a_star.c -o a_star.o

linked_list.o: linked_list.c tmrs.h tmrs_structs.h
	gcc ${CFLAGS} -c linked_list.c -o linked_list.o
	
utils.o: utils.c tmrs.h tmrs_structs.h
	gcc ${CFLAGS} -c utils.c -o utils.o 
	
map.o: map.c tmrs.h tmrs_structs.h
	gcc ${CFLAGS} -c map.c -o map.o 
        
server.o: server.c tmrs.h tmrs_structs.h
	gcc ${CFLAGS} -c server.c -o server.o 
	
clean:
	rm -f tmrs *.o
                                          
