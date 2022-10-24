#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>


int main()
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;

}
