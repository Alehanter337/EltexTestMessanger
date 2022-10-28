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
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

void config_parse(char *file_path)
{
    Lines = ParseConf(file_path);
    displayKeyValue(Lines);
}


int main(int argc, char* argv[])
{
    config_parse("src/config.conf");

    char *message;
    pid_t childpid;
    fd_set rset;
    struct sockaddr_in client, server;
    int listener = 0;
    char buff[BUFF_SIZE];
    int socket_desc = 0; 
    int port = 7331;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    
    socklen_t sockaddr_len = sizeof(struct sockaddr_in);
    
    /* Create TCP socket */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == ERROR)
    {
        perror("SERV_TCP_sock_err");
        exit(EXIT_FAILURE);
    }
    bzero(&server, sizeof(server));
    
    if (bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == ERROR)
    {
        perror("SERV_TCP_bind_err");
        exit(EXIT_FAILURE);
    }

    listen(listenfd, 10);

    /* Create UDP socket */
    int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpfd == ERROR)
    {
        perror("SERV_TCP_sock_err");
        exit(EXIT_FAILURE);
    }

    if (bind(udpfd, (struct sockaddr *)&server, sizeof(server)) == ERROR)
    {
        perror("SERV_UDP_bind_err");
        exit(EXIT_FAILURE);
    }
    
    FD_ZERO(&rset);

    int maxfd = MAX(listenfd, udpfd) + 1; 
    socklen_t len = sizeof(client);

    while (1)
    {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);

        int nready = select(maxfd, &rset, NULL, NULL, NULL);

        
        if (FD_ISSET(listenfd, &rset))
        {
            int connfd = accept(listenfd, (struct sockaddr*)&client, &len);
            if ((childpid = fork()) == 0)
            {
                close(listenfd);
                bzero(buff, BUFF_SIZE);
                
            
                //read(connfd, buff, BUFF_SIZE);
                puts(buff);

                while (1)
                {
                    int acpt = accept(listenfd, (struct sockaddr *)&server, &sockaddr_len);
                    if (acpt == ERROR)
                    {
                        perror("SERV_accept_err");
                        exit(EXIT_FAILURE);
                    }
                    int rcv = recv(acpt, buff, BUFF_SIZE, 0);
                    if (rcv == ERROR)
                    {
                        perror("Recvfrom error!");
                        exit(EXIT_FAILURE);
                    }
                printf("%s\n", buff);
                }





                //for send
                //write(connfd, (ddconst char*)message, BUFF_SIZE);
                close(connfd);
                exit(EXIT_FAILURE);
            }
            close(connfd);
        }

        if (FD_ISSET(udpfd, &rset)) 
        {
            bzero(buff, BUFF_SIZE);
            int recvv = recvfrom(udpfd, buff, BUFF_SIZE, 0,
                    (struct sockaddr*)&client, &len);
            puts(buff);
            
            //for send
            //sendto(udpfd, (const char*)message, sizeof(buff), 0,
            //        (struct sockaddr*)&client, sockaddr_len);
        }
    }



    /*
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


        while (1)
        {
            int acpt = accept(socket_desc, (struct sockaddr *)&server, &sockaddr_len);
            if (acpt == ERROR)
            {
                perror("SERV_accept_err");
                exit(EXIT_FAILURE);
            }
            int rcv = recv(acpt, buff, BUFF_SIZE, 0);
            if (rcv == ERROR)
            {
                perror("Recvfrom error!");
                exit(EXIT_FAILURE);
            }
            printf("%s\n", buff);
        }
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
    */
    freeConf(Lines);
}
