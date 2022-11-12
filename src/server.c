#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
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

struct message_cl
{
    char full_msg[BUFF_SIZE];
};

int recvd_udp_msg = 0;
int recvd_tcp_msg = 0;

char username[MAX_USER_LEN] = {0};
char username_f[MAX_USERF_LEN] = {0};
char inbox_username[MAX_USER_LEN] = {0};

char *server_address = {0};
char *log_level = {0};

struct sockaddr_in client, server, server_user;
socklen_t len = sizeof(client);

FILE *fp = NULL;

int get_count_of_threads()
{
    int threads_count = 0;
    char line[MAX_INBOX_LEN] = { 0 };
    FILE* fp = fopen("/proc/cpuinfo", "r");
    
    if (fp == NULL)
    {
        perror("fopen:/proc/cpuinfo");
        exit(EXIT_FAILURE);
    }

    while(fgets(line, MAX_INBOX_LEN, fp) != NULL)
    {
        sscanf(line, "processor\t: %d", &threads_count);
    }
    
    fclose(fp);

    threads_count++;

    return threads_count;
}

int get_hash(const char *s)
{
    const int n = strlen(s);
    const int p = 31, m = 1e9 + 7;
    int hash = 0;
    long p_pow = 1;
    for (int i = 0; i < n; i++)
    {
        hash = (hash + (s[i] - 'a' + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }
    return hash;
}

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
    inet_aton(server_address, &server_user.sin_addr);
    server_user.sin_port = htons(1337);

    if (bind(namefd, (struct sockaddr *)&server_user, sizeof(server_user)) == ERROR)
    {
        perror("name bind err");
        exit(EXIT_FAILURE);
    }
    bzero(user_message, MAX_USER_LEN);

    while (1)
    {
        char inbox_username[MAX_USER_LEN] = {0};
        /* recieve username when client connect */
        recvfrom(namefd, user_message, MAX_USER_LEN, 0,
                 (struct sockaddr *)&client, &len);

        if (strstr(user_message, "inbox") != NULL)
        {
            sscanf(user_message, "inbox %s", inbox_username);

            if (strcmp(log_level, "1") == EQUAL)
            {
                printf("%s request inbox\n", inbox_username);
            }
            sprintf(username_f, "clients_inbox/%s", inbox_username);

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
            bzero(inbox_username, MAX_USER_LEN);
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

void *handle_tcp_message(void *msg)
{
    char buffer[BUFF_SIZE] = {0};
    char message[BUFF_SIZE] = {0};
    char destination[MAX_USER_LEN] = {0};
    char user_group[MAX_USER_LEN] = {0};
    char *notif = {0};

    int delay = 0;
    int group_flag = 0;
    int clnt_hash_msg = 0;
    int serv_hash_msg = 0;

    strcpy(buffer, ((struct message_cl *)msg)->full_msg);
    
    if (strstr(buffer, "TOGROUP:") != NULL)
    {
        group_flag = 1;
        sscanf(buffer, "TOGROUP:%s\n%s\n%[^\t\n]%d",
               user_group,
               username,
               message,
               &clnt_hash_msg);
        strcpy(destination, user_group);
    }
    else
    {
        sscanf(buffer, "%s\n%s\n%[^\t\n]%d",
               destination,
               username,
               message,
               &clnt_hash_msg);
    }

    serv_hash_msg = get_hash(message);

    if (serv_hash_msg == clnt_hash_msg)
    {
        notif = "The message is intact";
    }
    else
    {
        notif = "The message is corruted";
    }
    if (strcmp(log_level, "1") == EQUAL)
    {
        puts(notif);
        printf("Message to %s\nFrom %s: %s\n",
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
    bzero(buffer, BUFF_SIZE);
    serv_hash_msg = 0;
    
}

int main(int argc, char *argv[])
{
    int arg = 0;
    if (argc <= NO_ARGS)
    {
        printf(RED "You need use ./server -c <conf/path> -i <ip.addr>\n");
        exit(EXIT_FAILURE);
    }
    while ((arg = getopt(argc, argv, "c:i:")) != -1)
    {
        switch (arg)
        {
        case 'c':
            puts("Using config");
            puts(optarg);
            config_parse(optarg);
            break;
        case 'i':
            puts("Choosen ip address interface");
            server_address = optarg;
            break;
        }
    }

    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);

    struct args *Args = (struct args *)malloc(sizeof(struct args));
    struct message_cl *Msg = (struct message_cl *)malloc(sizeof(struct message_cl));

    int delay = 0;
    int group_flag = 0;
    int clnt_hash_msg = 0;
    int serv_hash_msg = 0;


    int fd[2];
    int thread_count = get_count_of_threads();
    int curr_thrd = 0;

    char message[BUFF_SIZE] = {0};
    char destination[MAX_USER_LEN] = {0};
    char user_group[MAX_USER_LEN] = {0};

    char buff[BUFF_SIZE] = {0};

    pthread_t get_user;
    pthread_t clients[thread_count];

    pid_t childpid;

    fd_set rset;

    /* default socket's (TCP & UDP) for message settings */
    int port = 7331;
    server.sin_family = AF_INET;
    // server.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_aton(server_address, &server.sin_addr);
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

        select(maxfd, &rset, NULL, NULL, NULL);
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
            pipe(fd);
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
                close(fd[0]);
                write(fd[1], &buff, BUFF_SIZE);
                close(fd[1]);

                close(connfd);
                exit(EXIT_SUCCESS);
            }
            close(connfd);

            close(fd[1]);
            read(fd[0], &buff, BUFF_SIZE);
            close(fd[0]);

            strcpy(Msg->full_msg, buff);

            pthread_create(&clients[curr_thrd++], NULL, handle_tcp_message, (void *)Msg);
            if (curr_thrd < thread_count)
            {
                curr_thrd = 0;
            }
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
