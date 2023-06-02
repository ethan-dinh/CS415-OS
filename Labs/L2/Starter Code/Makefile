CFLAGS=-W -Wall
PROGRAMS=catf getword catchusr1 catchusr1v2

all: $(PROGRAMS)

catf: catf.o p1fxns.o
	gcc -o $@ $^

getword: getword.o p1fxns.o
	gcc -o $@ $^

catchusr1: catchusr1.o
	gcc -o $@ $^

catchusr1v2: catchusr1v2.o
	gcc -o $@ $^

p1fxns.o: p1fxns.c
catf.o: catf.c
getword.o: getword.c
catchusr1.o: catchusr1.c
catchusr1v2.o: catchusr1v2.c

clean:
	rm -f $(PROGRAMS) *.o
