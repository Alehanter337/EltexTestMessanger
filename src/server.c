#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include "ParseConf/ParseConf.c"

#define ERROR -1
#define BUFF_SIZE 256
#define MAX_USER_LEN 16
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

char username[MAX_USER_LEN];
struct sockaddr_in client, server, server_user;
socklen_t len = sizeof(client);

FILE *fp = NULL;

void config_parse(char *file_path)
{
    Lines = ParseConf(file_path);
    displayKeyValue(Lines);
}

void *get_user_func()
{
    int namefd = socket(AF_INET, SOCK_DGRAM, 0);    
    server_user.sin_family = AF_INET;
    server_user.sin_addr.s_addr = htonl(INADDR_ANY);
    server_user.sin_port = htons(1337);

    if (bind(namefd, (struct sockaddr *)&server_user, sizeof(server_user)) == ERROR)
    {
        perror("name bind err");
        exit(EXIT_FAILURE);
    }
    while(1)
    { 
        
        recvfrom(namefd, username, MAX_USER_LEN, 0,
            (struct sockaddr*)&client, &len);
        strcat(username, ".txt");
        fp = fopen(username, "a");

    }
    close(namefd);
}

void socket_for_username(pthread_t get_user)
{
    pthread_create(&get_user, NULL, get_user_func, NULL);
}


int main(int argc, char* argv[])
{
    config_parse("src/config.conf");

    char *message;
        
    pthread_t get_user;
    pid_t childpid;
    fd_set rset;
    char buff[BUFF_SIZE];
    int port = 7331;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);


    

    socket_for_username(get_user);
    socklen_t sockaddr_len = sizeof(struct sockaddr_in);
    
    /* Create TCP socket */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == ERROR)
    {
        perror("SERV_TCP_sock_err");
        exit(EXIT_FAILURE);
    }
    //bzero(&server, sizeof(server));
    
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

    while (1)
    {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);

        int nready = select(maxfd, &rset, NULL, NULL, NULL);

        
        if (FD_ISSET(listenfd, &rset))
        {
            int connfd = accept(listenfd, (struct sockaddr*)&client, &len);
            if (connfd == ERROR)
            {
                perror("SERV_accept_err");
                exit(EXIT_FAILURE);
            }
            if ((childpid = fork()) == 0)
            {
                close(listenfd);
                bzero(buff, BUFF_SIZE);
                 
                int rcv = recv(connfd, buff, BUFF_SIZE, 0);
                if (rcv == ERROR)
                {
                    perror("Recvfrom error!");
                    exit(EXIT_FAILURE);
                }
                printf("%s\n", buff);
                
                close(connfd);
                exit(EXIT_SUCCESS);
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
