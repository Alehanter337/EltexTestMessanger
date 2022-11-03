CC = gcc
CFLAGS = -pthread

default:
	$(CC) $(CFLAGS) src/client.c -o client
	$(CC) $(CFLAGS) src/server.c -o server

clean:
	rm -f client server clients/*
	echo "_" >> clients/client.test 
	cd groups
	rm -f groups/*
	touch groups/Alpha groups/Beta groups/Omega 