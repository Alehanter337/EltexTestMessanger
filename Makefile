CC = gcc
CFLAGS = -pthread

default:
	touch "List of clients"
	$(CC) $(CFLAGS) src/client.c -o client
	$(CC) $(CFLAGS) src/server.c -o server

clean:
	rm -f client server clients_inbox/*
	echo "_" >> clients_inbox/client.test 
	rm -f groups/*
	echo "_" >> groups/group.test
