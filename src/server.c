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
char username_f[BUFF_SIZE] = { 0 };
char* log_level = { 0 };

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
    Lines = ParseConf(file_path);
    log_level = Lines->value;
    if (strcmp(log_level, "0") == 0 || strcmp(log_level, "1") == 0 )
    {
        displayKeyValue(Lines);
        printf("----CONFIG IS OK----\n");
    }
    else 
    {
        printf("Bad config");
        exit(EXIT_FAILURE);
    }
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
        char inbox_buff[MAX_INBOX_LEN] = { 0 };
        char user_buff[MAX_USER_LEN] = { 0 };
        char inbox[MAX_INBOX_LEN] = { 0 };
        
        /* recieve username when client connect */
        recvfrom(namefd, username, MAX_USER_LEN, 0,
            (struct sockaddr*)&client, &len);

        if (strstr(username, "inbox") != 0)
        {
            left_to_var(username, user_buff);
            
            if (strcmp(log_level, "1") == EQUAL)
            {
                printf("%s request inbox\n", user_buff);  
            }
            sprintf(username_f, "clients/%s", user_buff);
            fp = fopen(username_f, "r");

            while((fgets(inbox, MAX_INBOX_LEN/2, fp)) != NULL)
            {          
                strcat(inbox_buff, inbox);
            }
            if (strcmp(log_level, "1") == EQUAL)
            {
                puts(inbox_buff);
            }
            fclose(fp);

            sendto(namefd, (const char*)inbox_buff, BUFF_SIZE, 0,
                (struct sockaddr*)&client, sizeof(client));

            bzero(inbox, MAX_INBOX_LEN);
            bzero(inbox_buff, MAX_INBOX_LEN);
            bzero(user_buff, MAX_USER_LEN);
            bzero(username_f, MAX_USER_LEN);
            bzero(username, MAX_USER_LEN);
        }
        else 
        {
            if (strcmp(log_level, "1") == EQUAL)
            {
                printf("\n%s connected to server!\n", username);
            }
            sprintf(username_f, "clients/%s", username);
            fp = fopen(username_f, "a");
            fclose(fp);
            bzero(username, MAX_USER_LEN);
            bzero(username_f, BUFF_SIZE);
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

    /* default socket's (TCP & UDP) for message settings */
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

    /* maximum num of fd ready */
    int maxfd = MAX(listenfd, udpfd) + 1; 

    while (1)
    {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);

        int nready = select(maxfd, &rset, NULL, NULL, NULL);

        /* wait first socket that start listen */        
        if (FD_ISSET(listenfd, &rset))
        {
            int connfd = accept(listenfd, (struct sockaddr*)&client, &sockaddr_len);
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

                if (strcmp(log_level, "1") == EQUAL)
                {
                    printf("Message to %s\nFrom %s", 
                        destination, 
                        buff + strlen(destination) + 1);
                }
                sprintf(username_f, "clients/%s", destination);
                fp = fopen(username_f, "a");
                
                /*delete from buff
                spaces and '='
                write to destination inbox file >_< */
                fprintf(fp, "%s", buff + strlen(destination) + 1);
                fclose(fp);
                bzero(destination, MAX_USER_LEN);
                bzero(buff, BUFF_SIZE);
                close(connfd);
            }
            close(connfd);
        }

        if (FD_ISSET(udpfd, &rset)) 
        {
            bzero(buff, BUFF_SIZE);
            bzero(destination, MAX_USER_LEN);
            int recvv = recvfrom(udpfd, buff, BUFF_SIZE, 0,
                    (struct sockaddr*)&client, &len);
            left_to_var(buff, destination);

            if (strcmp(log_level, "1") == EQUAL)
            {
                printf("Message to %s\nFrom %s", 
                    destination, 
                    buff + strlen(destination) + 1);
            }
            sprintf(username_f, "clients/%s", destination);
            fp = fopen(username_f, "a");
            /*
            delete from buff spaces and '='
            write to destination inbox file >_< 
            */
            fprintf(fp, "%s", buff + strlen(destination) + 1);
            fclose(fp);
            bzero(destination, MAX_USER_LEN);
            bzero(buff, BUFF_SIZE);
            close(udpfd);
        }
    }

    freeConf(Lines);
}
