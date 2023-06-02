CFLAGS=-W -Wall -g
LDFLAGS=-g
PROGRAMS=freelist boundedfreelist
LIBRARIES=-lpthread

all: $(PROGRAMS)

freelist: freelist.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

boundedfreelist: boundedfreelist.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

freelist.o: freelist.c
boundedfreelist.o: boundedfreelist.c

clean:
	rm -f $(PROGRAMS) *.o
