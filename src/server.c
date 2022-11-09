#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>

#include "ParseConf/ParseConf.c"
#include "server.h"

struct args
{
    char username[MAX_USER_LEN];
    char username_f[MAX_USERF_LEN];
    char message[MAX_USER_LEN];
    int delay;
};

int recvd_udp_msg = 0;
int recvd_tcp_msg = 0;

char username[MAX_USER_LEN] = {0};
char username_f[MAX_USERF_LEN] = {0};

char *log_level = {0};

struct sockaddr_in client, server, server_user;
socklen_t len = sizeof(client);

FILE *fp = NULL;

char *str_remove(char *str, const char *sub)
{
    size_t len = strlen(sub);
    if (len > 0)
    {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL)
        {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

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
    if (strcmp(log_level, "0") == 0 || strcmp(log_level, "1") == 0)
    {
        displayKeyValue(Lines);
        printf("----CONFIG IS OK----\n");
    }
    else
    {
        if (strcmp(log_level, "1") == EQUAL)
        {
            printf("Bad config");
        }

        exit(EXIT_FAILURE);
    }
}

void *send_with_delay(void *arg)
{
    sleep(((struct args *)arg)->delay);
    fp = fopen(((struct args *)arg)->username_f, "a");

    if (fp == NULL)
    {
        perror("Cannot open file");
    }
    else
    {
        if (strcmp(log_level, "1") == EQUAL)
        {
            printf("Message with delay %d sec. to %s\nFrom %s: %s\n",
                   ((struct args *)arg)->delay,
                   ((struct args *)arg)->username_f + DEL_CLIENTS_LEN,
                   ((struct args *)arg)->username,
                   ((struct args *)arg)->message);
        }
        fprintf(fp, "%s: %s\n", ((struct args *)arg)->username, ((struct args *)arg)->message);
    }
    fclose(fp);
    return 0;
}

void *get_user_func()
{
    char inbox_buff[MAX_INBOX_LEN] = {0};
    char inbox[MAX_INBOX_LEN] = {0};
    char group[MAX_USER_LEN] = {0};
    char user_message[BUFF_SIZE] = {0};
    char list_of_clients[MAX_INBOX_LEN] = {0};
    char delete_sub_buff[MAX_INBOX_LEN] = {0};
    char user_plus_group[MAX_USERF_LEN] = {0};

    int namefd = socket(AF_INET, SOCK_DGRAM, 0);

    server_user.sin_family = AF_INET;
    server_user.sin_addr.s_addr = htonl(INADDR_ANY);
    server_user.sin_port = htons(1337);

    if (bind(namefd, (struct sockaddr *)&server_user, sizeof(server_user)) == ERROR)
    {
        perror("name bind err");
        exit(EXIT_FAILURE);
    }
    bzero(user_message, MAX_USER_LEN);

    while (1)
    {
        /* recieve username when client connect */
        recvfrom(namefd, user_message, MAX_USER_LEN, 0,
                 (struct sockaddr *)&client, &len);

        if (strstr(user_message, "inbox") != 0)
        {
            left_to_var(user_message, username);

            if (strcmp(log_level, "1") == EQUAL)
            {
                printf("%s request inbox\n", username);
            }
            sprintf(username_f, "clients_inbox/%s", username);

            fp = fopen(username_f, "r");
            if (!fp)
            {
                if (strcmp(log_level, "1") == EQUAL)
                {
                    puts("No such inbox file");
                }
                strcat(inbox_buff, "No such inbox file");
            }
            else
            {
                while ((fgets(inbox, MAX_INBOX_LEN / 2, fp)) != NULL)
                {
                    strcat(inbox_buff, inbox);
                }
                if (strcmp(log_level, "1") == EQUAL)
                {
                    puts(inbox_buff);
                }
                fclose(fp);
            }
            sendto(namefd, (const char *)inbox_buff, BUFF_SIZE, 0,
                   (struct sockaddr *)&client, sizeof(client));

            bzero(inbox, MAX_INBOX_LEN);
            bzero(inbox_buff, MAX_INBOX_LEN);
            bzero(user_message, BUFF_SIZE);
            bzero(username_f, MAX_USERF_LEN);
            bzero(username, MAX_USER_LEN);
        }
        else
        {
            sscanf(user_message, "%s - %s", username, group);

            if (strcmp(log_level, "1") == EQUAL)
            {
                printf("\n%s - %s on server!\n", username, group);
            }

            sprintf(username_f, "clients_inbox/%s", username);

            fp = fopen(username_f, "a");
            fclose(fp);

            fp = fopen("List of clients", "r");

            while ((fgets(list_of_clients, MAX_INBOX_LEN / 2, fp)) != NULL)
            {
                if (strstr(list_of_clients, username) == NULL)
                {
                    strcat(delete_sub_buff, list_of_clients);
                }
            }
            fclose(fp);
            fp = fopen("List of clients", "w");

            sprintf(user_plus_group, "%s - ", username);
            strcat(user_plus_group, group);

            strcat(delete_sub_buff, user_plus_group);
            fprintf(fp, "%s", delete_sub_buff);

            fclose(fp);

            bzero(delete_sub_buff, MAX_INBOX_LEN);
            bzero(username, MAX_USER_LEN);
            bzero(user_message, BUFF_SIZE);
            bzero(group, MAX_USER_LEN);
            bzero(username_f, MAX_USERF_LEN);
            bzero(user_plus_group, MAX_USERF_LEN);
        }
    }
    close(namefd);
}

void socket_for_username(pthread_t get_user)
{
    pthread_create(&get_user, NULL, get_user_func, NULL);
}

void handler_sigusr1(int sig)
{
    printf("\n%d UDP recieved messages", recvd_udp_msg);
    printf("\n%d TCP recieved messages\n", recvd_tcp_msg);
}

void handler_sigusr2(int sig)
{
    char file_contain[MAX_INBOX_LEN] = {0};
    char buffer[MAX_INBOX_LEN] = {0};

    puts("\nGroup statistic\n");
    puts("|Username - Group|");
    fp = fopen("List of clients", "r");

    while ((fgets(file_contain, MAX_INBOX_LEN / 2, fp)) != NULL)
    {
        strcat(buffer, file_contain);
    }
    puts(buffer);
    bzero(file_contain, MAX_INBOX_LEN);
    bzero(buffer, MAX_INBOX_LEN);
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        puts("Using custom config");
        for (int i = 0; i < argc; i++)
        {
            if (strcmp(argv[i], "-c") == 0)
            {
                config_parse(argv[i + 1]);
            }
        }
    }
    else
    {
        puts("Using default config");
        config_parse("src/config.conf");
    }

    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);

    struct args *Args = (struct args *)malloc(sizeof(struct args));
    int delay = 0;
    int group_flag = 0;

    char message[BUFF_SIZE] = {0};
    char destination[MAX_USER_LEN] = {0};
    char user_group[MAX_USER_LEN] = {0};
    char buff[BUFF_SIZE] = {0};

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
            int connfd = accept(listenfd, (struct sockaddr *)&client, &len);
            if (connfd == ERROR)
            {
                perror("SERV_accept_err");
                exit(EXIT_FAILURE);
            }
            recvd_tcp_msg++;

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

                if (strstr(buff, "TOGROUP:") != NULL)
                {
                    group_flag = 1;
                    sscanf(buff, "TOGROUP:%s\n%s\n%[^\t\n]",
                           user_group,
                           username,
                           message);
                    strcpy(destination, user_group);
                }
                else
                {
                    sscanf(buff, "%s\n%s\n%[^\t\n]",
                           destination,
                           username,
                           message);
                }

                if (strcmp(log_level, "1") == EQUAL)
                {
                    printf("\nMessage to %s\nFrom %s: %s\n",
                           destination,
                           username,
                           message);
                }

                sprintf(username_f, "clients_inbox/%s", destination);
                fp = fopen(username_f, "a");
                fprintf(fp, "%s: %s\n", username, message);
                fclose(fp);
                bzero(destination, MAX_USER_LEN);
                bzero(message, BUFF_SIZE);
                bzero(username_f, MAX_USERF_LEN);
                bzero(buff, BUFF_SIZE);

                close(connfd);
                exit(EXIT_SUCCESS);
            }
            close(connfd);
        }

        if (FD_ISSET(udpfd, &rset))
        {
            int recvv = recvfrom(udpfd, buff, BUFF_SIZE, 0,
                                 (struct sockaddr *)&client, &len);
            recvd_udp_msg++;
            if (strstr(buff, "DELAY:") != NULL)
            {
                pthread_t delay_sender;
                sscanf(buff, "DELAY:%d\n%s\n%s\n%[^\t\n]",
                       &delay,
                       destination,
                       username,
                       message);

                Args->delay = delay;
                strcat(Args->username, username);
                sprintf(Args->username_f, "clients_inbox/%s", destination);
                strcat(Args->message, message);
                pthread_create(&delay_sender, NULL, send_with_delay, (void *)Args);
                free(Args);
            }
            else
            {
                if (strstr(buff, "TOGROUP:") != NULL)
                {
                    group_flag = 1;
                    sscanf(buff, "TOGROUP:%s\n%s\n%[^\t\n]",
                           user_group,
                           username,
                           message);
                    strcpy(destination, user_group);
                }
                else
                {
                    sscanf(buff, "%s\n%s\n%[^\t\n]",
                           destination,
                           username,
                           message);
                }

                if (strcmp(log_level, "1") == EQUAL)
                {
                    printf("\nMessage to %s\nFrom %s: %s\n", destination, username, message);
                }
            }

            sprintf(username_f, "clients_inbox/%s", destination);
            fp = fopen(username_f, "a");
            fprintf(fp, "%s: %s\n", username, message);
            fclose(fp);

            bzero(destination, MAX_USER_LEN);
            bzero(message, BUFF_SIZE);
            bzero(username_f, MAX_USERF_LEN);
            bzero(buff, BUFF_SIZE);
        }
    }
}
