#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>

#include "ParseConf/ParseConf.c"
#include "server.h"


char username[MAX_USER_LEN] = { 0 };

struct sockaddr_in client, server, server_user;
socklen_t len = sizeof(client);

FILE *fp = NULL;

void left_to_var(char buff[BUFF_SIZE], char destination[MAX_USER_LEN])
{
    int i = 0;
    while (buff[i] != '=')
    {
        destination[i] += buff[i];
        buff[i] = ' ';
        i++;
    }
}

void config_parse(char *file_path)
{
    int dot = 0;
    Lines = ParseConf(file_path);
    printf("----CONFIG IS OK----\n");
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
        char inbox_buff[MAX_INBOX_LEN];
        char user_buff[MAX_USER_LEN];
        recvfrom(namefd, username, MAX_USER_LEN, 0,
            (struct sockaddr*)&client, &len);

        if (strstr(username, "inbox") != 0)
        {
            left_to_var(username, user_buff);
            char inbox[MAX_INBOX_LEN] = { 0 };

            printf("%s request inbox\n", user_buff);  
            fp = fopen(user_buff, "r");
            while((fgets(inbox, MAX_INBOX_LEN/4, fp)) != NULL)
            {          
                strcat(inbox_buff, inbox);
            }
            puts(inbox_buff);
            fclose(fp);

            sendto(namefd, (const char*)inbox_buff, BUFF_SIZE, 0,
                (struct sockaddr*)&client, sizeof(client));

            bzero(inbox, MAX_INBOX_LEN);
            bzero(inbox_buff, MAX_INBOX_LEN);
            bzero(user_buff, MAX_USER_LEN);
            bzero(username, MAX_USER_LEN);
        }
        else 
        {
            fp = fopen(username, "a");
            fclose(fp);
            bzero(username, MAX_USER_LEN);
        }
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

    char message[BUFF_SIZE] = { 0 };
    char destination[MAX_USER_LEN] = { 0 };
    char buff[BUFF_SIZE] = { 0 };

    pthread_t get_user;

    pid_t childpid;

    fd_set rset;

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

                int rcv = recv(connfd, buff, BUFF_SIZE, 0);
                if (rcv == ERROR)
                {
                    perror("Recvfrom error!");
                    exit(EXIT_FAILURE);
                }

                left_to_var(buff, destination);

                printf("Message to %s\nFrom %s", 
                            destination, 
                            buff + strlen(destination) + 1);

                fp = fopen(destination, "a");

                /*delete from buff
                spaces and '='
                write to destination inbox file >_< */
                fprintf(fp, "%s", buff + strlen(destination) + 1);
                fclose(fp);
                bzero(destination, MAX_USER_LEN);
                bzero(buff, BUFF_SIZE);
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
