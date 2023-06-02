CFLAGS=-W -Wall -I/usr/local/include
LDFLAGS=-L/usr/local/lib
LIBRARIES=-lADTs
PROGRAMS= stubtest

all: $(PROGRAMS)

stubtest: stubtest.o arraystub.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

stubtest.o: stubtest.c
arraystub.o: arraystub.c

clean:
	rm -f $(PROGRAMS) *.o
