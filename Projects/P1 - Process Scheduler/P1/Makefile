CFLAGS=-W -Wall -g -I/usr/local/include -g
LDFLAGS=-L/usr/local/lib
LIBRARIES=-lADTs
PROGRAMS= uspsv1 uspsv2 uspsv3 uspsv4 iobound cpubound 

all: $(PROGRAMS)

uspsv1: uspsv1.o p1fxns.o
	gcc -o $@ $^
uspsv2: uspsv2.o p1fxns.o
	gcc -o $@ $^
uspsv3: uspsv3.o p1fxns.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)
uspsv4: uspsv4.o p1fxns.o pidDisplay.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)
iobound: iobound.o
	gcc -o $@ $^
cpubound: cpubound.o
	gcc -o $@ $^

p1fxns.o: p1fxns.c
pidDisplay.o: pidDisplay.c
uspsv1.o: uspsv1.c
uspsv2.o: uspsv2.c
uspsv3.o: uspsv3.c
uspsv4.o: uspsv4.c
iobound.o: iobound.c
cpubound.o: cpubound.c

clean:
	rm -f $(PROGRAMS) *.o
