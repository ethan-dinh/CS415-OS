CFLAGS=-W -Wall -pedantic -I/usr/local/include
LDFLAGS=-L/usr/local/lib
PROGRAMS=client server cbclient cbserver
LIBRARIES=-lBXP -lpthread

all: $(PROGRAMS)

client: client.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

cbclient: cbclient.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

server: server.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

cbserver: cbserver.o
	gcc $(LDFLAGS) -o $@ $^ $(LIBRARIES)

client.o: client.c
cbclient.o: cbclient.c
server.o: server.c
cbserver.o: cbserver.c

clean:
	rm -f $(PROGRAMS) *.o
