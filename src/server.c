#include <asm-generic/socket.h>
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

#include "ParseConf/ParseConf.c"

#define ERROR -1
#define BUFF_SIZE 256

void config_parse(char *file_path)
{
    Lines = ParseConf(file_path);
    displayKeyValue(Lines);
}


int main(int argc, char* argv[])
{
    config_parse("src/config.conf");

    struct sockaddr_in server;
    int listener = 0;
    char buff[BUFF_SIZE];
    int socket_desc = 0; 
    int port = 7331;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    
    socklen_t sockaddr_len = sizeof(struct sockaddr_in);
 
    

    if (strcmp(getValue(Lines, "TCP"),"yes") == 0 ||
        strcmp(getValue(Lines, "TCP"),"1") == 0 )
    {
        socket_desc = socket(AF_INET, SOCK_STREAM, 0); //TCP
        
        int serv = bind(socket_desc, (struct sockaddr *)&server, sockaddr_len);
        if (serv == ERROR)
        {
            perror("SERV_bind_err");
            exit(EXIT_FAILURE);
        }

        if (listen(socket_desc, 1) < 0) 
        {
            perror("SERV_listen_err");
            exit(EXIT_FAILURE);
        }

        int acpt = accept(socket_desc, (struct sockaddr *)&server, &sockaddr_len);
        if (acpt == ERROR)
        {
            perror("SERV_accept_err");
            exit(EXIT_FAILURE);
        }

        while (1)
        {
            int rcv = recv(acpt, buff, BUFF_SIZE, 0);
            if (rcv == ERROR)
            {
                perror("Recvfrom error!");
                exit(EXIT_FAILURE);
            }
            printf("%s\n", buff);
        }
        printf("%s\n", buff);
    }
     
    else if (strcmp(getValue(Lines, "UDP"),"yes") == 0 ||
             strcmp(getValue(Lines, "UDP"),"1") == 0 )
    {
        socket_desc = socket(AF_INET, SOCK_DGRAM, 0); //UDP
        if (socket_desc == ERROR)
        {
            perror("SERVER: Socket error!");
            exit(EXIT_FAILURE);
        }
    
        int serv = bind(socket_desc, (struct sockaddr *)&server, sockaddr_len);
        if (serv == ERROR)
        {
            perror("SERVER: Bind error!");
            exit(EXIT_FAILURE);
        }
    
        while (1)
        {
            int rcv = recvfrom(socket_desc, buff, BUFF_SIZE, 0, NULL, NULL);
            if (rcv == ERROR)
            {
                perror("Recvfrom error!");
                exit(EXIT_FAILURE);
            }
            printf("%s\n", buff);
        }
    }
    close(socket_desc);
    freeConf(Lines);
}
