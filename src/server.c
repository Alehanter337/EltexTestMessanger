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
#include <ctype.h>
#include <arpa/inet.h>

#include "ParseConf/ParseConf.c"

#define ERROR -1
#define BUFF_SIZE 256
#define MAX_USER_LEN 16
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

char username[MAX_USER_LEN] = { 0 };
char destination[MAX_USER_LEN] = { 0 };
struct sockaddr_in client, server, server_user, server_dest;
socklen_t len = sizeof(client);

FILE *fp = NULL;


void config_parse(char *file_path)
{
    int dot = 0;
    Lines = ParseConf(file_path);
    printf("----CONFIG IS OK-----\n");
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
    bzero(username, MAX_USER_LEN);
    while(1)
    { 
        recvfrom(namefd, username, MAX_USER_LEN, 0,
            (struct sockaddr*)&client, &len);
        //strcat(username, ".inbox");
        fp = fopen(username, "a");
        fclose(fp);
        bzero(username, MAX_USER_LEN);
    }
    close(namefd);
}

void *get_dest_func()
{
    int destfd = socket(AF_INET, SOCK_DGRAM, 0);    
    server_dest.sin_family = AF_INET;
    server_dest.sin_addr.s_addr = htonl(INADDR_ANY);
    server_dest.sin_port = htons(1338);

    if (bind(destfd, (struct sockaddr *)&server_dest, sizeof(server_dest)) == ERROR)
    {
        perror("name bind err");
        exit(EXIT_FAILURE);
    }
    bzero(destination, MAX_USER_LEN);
    while(1)
    { 
        recvfrom(destfd, destination, MAX_USER_LEN, 0,
            (struct sockaddr*)&client, &len);
        fp = fopen(destination, "a");
        fclose(fp);

    }
    close(destfd);
}


void socket_for_username(pthread_t get_user)
{
    pthread_create(&get_user, NULL, get_user_func, NULL);
}

void socket_for_destination(pthread_t get_dest)
{
    pthread_create(&get_dest, NULL, get_dest_func, NULL);
}


int main(int argc, char* argv[])
{
    config_parse("src/config.conf");

    char *message = NULL;

    pthread_t get_user;
    pthread_t get_dest;
    pid_t childpid;
    fd_set rset;
    char buff[BUFF_SIZE] = { 0 };
    int port = 7331;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);


    socket_for_username(get_user);
    socket_for_destination(get_dest);
    socklen_t sockaddr_len = sizeof(struct sockaddr_in);
    
    /* Create TCP socket */
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == ERROR)
    {
        perror("SERV_TCP_sock_err");
        exit(EXIT_FAILURE);
    }
    
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
                /*int ctr = 0;
                 Get destination nickname 
                printf("Recieved message\nTo: ");
                while(isprint(buff[ctr]))
                {   
                    char buff_char = putchar(buff[ctr]);
                    strcat(destination, &buff_char);
                    ctr++;
                }*/

                //strcat(destination, ".inbox");

                printf("Message recieved\n| Username: Message | \n  %s", buff);
                
                fp = fopen(destination, "a");
                fprintf(fp, "%s", buff);
                fclose(fp);
                bzero(destination, MAX_USER_LEN);
                close(connfd);
                exit(EXIT_SUCCESS);
            }
            close(connfd);
        }

        if (FD_ISSET(udpfd, &rset)) 
        {
            bzero(buff, BUFF_SIZE);
            bzero(destination, MAX_USER_LEN);
            int recvv = recvfrom(udpfd, buff, BUFF_SIZE, 0,
                    (struct sockaddr*)&client, &len);
            int ctr = 0;
            /* Get destination nickname */
            printf("Recieved message\nTo: ");
            while(isprint(buff[ctr]))
            {   
                char buff_char = putchar(buff[ctr]);
                strcat(destination, &buff_char);
                ctr++;
            }
            printf("\n");

            printf("%s", buff + strlen(destination) - strlen(username));            
            //for send
            //sendto(udpfd, (const char*)message, sizeof(buff), 0,
            //        (struct sockaddr*)&client, sockaddr_len);
        }
    }

    freeConf(Lines);
}
