#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <arpa/inet.h>

#include "ParseConf/ParseConf.c"
#include "server.h"

FILE *fp = NULL;

// char username[MAX_USER_LEN] = {0};
char username_f[MAX_USERF_LEN] = {0};
char *username = {0};
char group[MAX_USER_LEN] = {0};
char group_f[MAX_USERF_LEN] = {0};

char user_plus_group[MAX_USERF_LEN] = {0};

void clean_choice()
{
    char s;
    do
    {
        s = getchar();
    } while ('\n' != s && EOF != s);
}

void help()
{
    printf(RED "Please run:\n./client -s <server.ip.address> -u <yourUsername>\n");
}

void print_menu()
{
    printf("\nUser: %s\n", username);
    printf("Group: \"%s\"\n", group);
    printf("\nChoose action:\n");
    printf("1 - Check inbox\n");
    printf("2 - Send message\n");
    printf("3 - Join the group\n");
    printf("4 - Leave the group\n");
    printf("5 - Exit\n");
}

int main(int argc, char *argv[])
{
    struct sockaddr_in server, server_user;

    int socket_desc = 0;
    int group_choose = 0;
    int action = 0;
    int message_choose = 0;
    int group_flag = 0;
    int group_inbox_flag = 0;
    int delay = 0;
    int arg = 0;

    char *server_address = {0};
    char destination[MAX_USER_LEN] = {0};
    char message[BUFF_SIZE] = {0};
    char message_nick[BUFF_SIZE] = {0};
    char inbox_out[MAX_INBOX_LEN] = {0};
    char inbox_req[BUFF_SIZE] = {0};

    if (argc <= NO_ARGS)
    {
        help();
        return 0;
    }

    while ((arg = getopt(argc, argv, "u:s:")) != -1)
    {
        switch (arg)
        {
        case 'u':
            username = optarg;
            if (strcmp(username, " ") == 0)
            {
                puts("Error: Username is empty!");
                exit(EXIT_FAILURE);
            }
            break;
        case 's':
            server_address = optarg;
            break;
        }
    }

    sprintf(username_f, "groups/%s", username);
    fp = fopen(username_f, "r");
    // check exist file user with group
    if (!fp)
    {
        fp = fopen(username_f, "w");
        fprintf(fp, "%s", "NoGroup");
        strcpy(group, "NoGroup");
    }
    else
    {
        fp = fopen(username_f, "r");
        while ((fgets(group_f, MAX_USER_LEN, fp)) != NULL)
        {
            strcpy(group, group_f);
        }
    }
    fclose(fp);
    server.sin_family = AF_INET;
    inet_aton(server_address, &server.sin_addr);
    server.sin_port = htons(7331);

    server_user.sin_family = AF_INET;
    server_user.sin_port = htons(1337);
    inet_aton(server_address, &server_user.sin_addr);

    sprintf(user_plus_group, "%s - ", username);
    strcat(user_plus_group, group);

    int namefd = socket(AF_INET, SOCK_DGRAM, 0);
    sendto(namefd, (const char *)user_plus_group, strlen(user_plus_group), 0,
           (const struct sockaddr *)&server_user, sizeof(server_user));

    bzero(user_plus_group, MAX_USERF_LEN);

    print_menu();

    while (1)
    {
        scanf("%i", &action);

        switch (action)
        {
        case 1:

            printf("Check inbox\n");

            if (strcmp(group, "NoGroup") != 0)
            {
                puts("Check group inbox?\n1 - yes \n2 - no");

                scanf("%d", &group_inbox_flag);
                if (group_inbox_flag == 1)
                {
                    sprintf(inbox_req, "inbox %s", group);
                }
                else
                {
                    sprintf(inbox_req, "inbox %s", username);
                }
            }
            else
            {
                sprintf(inbox_req, "inbox %s", username);
            }
            
            int inboxfd = socket(AF_INET, SOCK_DGRAM, 0);

            sendto(inboxfd, (const char *)inbox_req, strlen(inbox_req), 0,
                   (const struct sockaddr *)&server_user, sizeof(server_user));
            socklen_t len = sizeof(server_user);

            printf("|Username: Message|\n");
            recvfrom(inboxfd, (char *)inbox_out, MAX_INBOX_LEN,
                     0, (struct sockaddr *)&server_user, &len);

            puts(inbox_out);

            close(inboxfd);

            print_menu();
            break;

        case 2:
            printf("\n1 - Send message with delivery guarantee (TCP)\n");
            printf("2 - Send message without delivery guarantee (UDP)\n");
            printf("3 - Send message with delay (UDP)\n");
            scanf(" %i", &message_choose);

            if (message_choose == 1)
            {
                printf("\nTCP\n");
                socket_desc = socket(AF_INET, SOCK_STREAM, 0);
            }

            if (message_choose == 2)
            {
                socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
                printf("\nUDP\n");
            }

            if (message_choose == 3)
            {
                socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
                printf("\nUDP\nEnter delay: ");
                scanf("%d", &delay);

                while (delay < 0)
                {
                    printf("\nEnter corrent delay > 0\n");
                    scanf("%d", &delay);
                }
            }
            if (strcmp(group, "NoGroup") != 0)
            {
                puts("Send message to group?\n1 - Yes\n2 - No");
                scanf("%d", &group_flag);
            }

            printf("Enter your message: ");
            clean_choice(); // delete splitting message
            fgets(message, BUFF_SIZE, stdin);

            if (group_flag != 1)
            {
                printf("Enter destination: ");
                fgets(destination, MAX_USER_LEN, stdin);
                destination[strlen(destination) - 1] = '\0';
            }

            if (message_choose == 3)
            {
                sprintf(message_nick, "DELAY:%d\n%s\n%s\n", delay, destination, username);
            }

            else
            {
                if (group_flag == 1)
                {
                    sprintf(message_nick, "TOGROUP:%s\n%s\n", group, username);
                }
                else
                {
                    sprintf(message_nick, "%s\n%s\n", destination, username);
                }
            }
            strcat(message_nick, message);
            puts(message_nick);
            if (socket_desc == ERROR)
            {
                perror("CL_Socket_err");
                exit(EXIT_FAILURE);
            }

            socklen_t sockaddr_len = sizeof(struct sockaddr_in);
            int serv = connect(socket_desc, (struct sockaddr *)&server, sockaddr_len);

            if (serv == ERROR)
            {
                perror("CL_Connect_err");
                print_menu();
                break;
            }

            printf("\n%s\n", message_nick);

            int snd = send(socket_desc, message_nick, BUFF_SIZE, 0);
            if (snd == ERROR)
            {
                perror("CL_Send_err");
                exit(EXIT_FAILURE);
            }
            bzero(message_nick, BUFF_SIZE);
            bzero(message, BUFF_SIZE);
            message_choose = 0;
            group_flag = 0;
            close(socket_desc);
            print_menu();
            break;

        case 3:
            printf("\nJoin the group\n");
            printf("Choose group:\n");
            printf("1 - Alpha\n");
            printf("2 - Beta\n");
            printf("3 - Omega\n");

            if (strcmp(group, "NoGroup") != 0) // if group != "NoGroup"
            {
                printf("\nYou need to leave your group \"%s\" first!\n", group);
                sleep(1);
                print_menu();
                break;
            }

            scanf("%i", &group_choose);

            if (group_choose == 1)
            {
                strcpy(group, "Alpha");
            }

            else if (group_choose == 2)
            {
                strcpy(group, "Beta");
            }

            else if (group_choose == 3)
            {
                strcpy(group, "Omega");
            }

            else
            {
                printf("Incorrect number!\nBack to menu\n<-\n");
                print_menu();
                break;
            }

            fp = fopen(username_f, "w");
            fprintf(fp, "%s", group);
            fclose(fp);
            sprintf(user_plus_group, "%s - ", username);
            strcat(user_plus_group, group);

            sendto(namefd, (const char *)user_plus_group, strlen(user_plus_group), 0,
                   (const struct sockaddr *)&server_user, sizeof(server_user));

            bzero(user_plus_group, MAX_USERF_LEN);
            printf("\nChoose group \"%s\"\n", group);
            print_menu();
            break;

        case 4:
            printf("\nLeave the group\n");

            if (strcmp(group, "NoGroup") == 0)
            {
                printf("Not in group now\n");
                print_menu();
                break;
            }

            printf("Leaved from \"%s\"\n", group);
            fp = fopen(username_f, "w");
            fprintf(fp, "NoGroup");
            fclose(fp);
            strcpy(group, "NoGroup");
            sprintf(user_plus_group, "%s - ", username);
            strcat(user_plus_group, group);

            sendto(namefd, (const char *)user_plus_group, strlen(user_plus_group), 0,
                   (const struct sockaddr *)&server_user, sizeof(server_user));

            print_menu();
            break;

        case 5:
            close(namefd);
            return 0;
        }
    }
    close(namefd);
}
