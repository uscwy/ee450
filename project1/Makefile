
CC=g++
CFLAGS=-g -Wall
LDFLAGS=

all: server client
    
server: server.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@
	
client:	client.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -f server client
