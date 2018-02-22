
CC=g++
CFLAGS=-g -Wall
LDFLAGS=-pthread

all: server client

server: server.cpp

client:	client.cpp

.cpp:
	$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f server client
