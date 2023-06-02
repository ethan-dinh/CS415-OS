PROGRAMS=forktest exectest execdoit forkdoit

all: $(PROGRAMS)

forktest: forktest.o
	gcc -o $@ $^

exectest: exectest.o
	gcc -o $@ $^

execdoit: execdoit.o
	gcc -o $@ $^

forkdoit: forkdoit.o
	gcc -o $@ $^

forktest.o: forktest.c
exectest.o: exectest.c
forkdoit.o: forkdoit.c
execdoit.o: execdoit.c

clean:
	rm -f $(PROGRAMS) *.o
