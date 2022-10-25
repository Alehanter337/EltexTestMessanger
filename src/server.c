#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

int main()
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("10.0.0.1");
    server.sin_port = htons(80);
    
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0); //TCP

    if (socket_desc == -1)
    {
        perror("Socket error!");
        exit(EXIT_FAILURE);
    }


}
