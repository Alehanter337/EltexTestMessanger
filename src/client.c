#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ParseConf/ParseConf.c"

#define NO_ARGS 1
#define MAX_ADDR_LEN 16
#define MAX_USER_LEN 16
#define ERROR -1
#define BUFF_SIZE 512

void clean_choice()
{
    char s;
    do 
    {
        s = getchar();
    }
    while ('\n' != s && EOF != s);
}

void help()
{
    printf("Please run:\n./client -s \"server_address\" -u \"your_name\"\n");
}

void print_menu()
{
    printf("\nChoose action:\n");
    printf("1 - Check inbox\n");
    printf("2 - Send message\n");
    printf("3 - Show message delivery status\n");
    printf("4 - Join the group\n");
    printf("5 - Leave the group\n");
    printf("6 - Exit\n");
}

int main(int argc, char *argv[])  
{   
    struct sockaddr_in server, server_user, server_dest;
    
    int listener = 0, socket_desc = 0;
    char server_address[MAX_ADDR_LEN] = { 0 };
    char username[MAX_USER_LEN] = ""; 
    char destination[MAX_USER_LEN] = { 0 };
    char group[MAX_USER_LEN] = { 0 };
    char message[BUFF_SIZE] = { 0 };
    char message_nick[BUFF_SIZE] = { 0 };
    
     

    int group_choose = 0;
    int action = 0;
    int message_choose = 0;

    if (argc <= NO_ARGS) 
    {
        help();
        return 0;
    }
    
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-s") == 0)
        {
            strcat(server_address, argv[i+1]);
        }
    
        else if (strcmp(argv[i], "-u") == 0)
        {
            strcat(username, argv[i+1]);
        }
    }

    server.sin_family = AF_INET;
    inet_aton(server_address, &server.sin_addr);
    //server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(7331);
    
    server_user.sin_family = AF_INET;
    server_user.sin_port = htons(1337);
    inet_aton(server_address, &server_user.sin_addr);
    //server_user.sin_addr.s_addr = htonl(INADDR_ANY);

    server_dest.sin_family = AF_INET;
    server_dest.sin_port = htons(1338);
    inet_aton(server_address, &server_dest.sin_addr);

    printf("Hello, %s!\n", username);

    int namefd = socket(AF_INET, SOCK_DGRAM, 0);
    sendto(namefd, (const char*)username, strlen(username), 0,
            (const struct sockaddr*)&server_user, sizeof(server_user));
    close(namefd);    

    print_menu();

    while(1)
    {
        scanf("%i", &action);
        
        switch(action) 
        {
            case 1:
                printf("Check inbox\n");
                break;

            case 2: 
                printf("\n1 - Send message with delivery guarantee (TCP)\n");
                printf("2 - Send message without delivery guarantee (UDP)\n");
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

                printf("Enter your message: ");
                
                clean_choice(); 
                fgets(message, BUFF_SIZE, stdin); 
                printf("Enter destination: ");
                fgets(destination, MAX_USER_LEN, stdin); 
                destination[ strlen(destination)-1 ] = '\0';
                memset(message_nick, 0, BUFF_SIZE);
                sprintf(message_nick, "%s=%s: %s", destination, username, message);
                /*strcat(message_nick, destination);
                strcat(message_nick, )
                strcat(message_nick, username);
                strcat(message_nick, ": ");
                strcat(message_nick, message);*/
                

                memset(message, 0, BUFF_SIZE);
                
                if (socket_desc == ERROR)
                {
                    perror("CL_Socket_err");
                    exit(EXIT_FAILURE);
                }

                socklen_t sockaddr_len = sizeof(struct sockaddr_in);
                int serv = connect(socket_desc, (struct sockaddr *) &server, sockaddr_len);

                if (serv == ERROR)
	            {
	                perror("CL_Connect_err");
	                print_menu();
                    break;
                } 

                printf("\n\n%s\n\n", message_nick);
                /*
                int destfd = socket(AF_INET, SOCK_DGRAM, 0);
                sendto(destfd, (const char*)destination, strlen(destination), 0,
                        (const struct sockaddr*)&server_dest, sizeof(server_dest));
                close(destfd);
                */
            
	            int snd = send(socket_desc, message_nick, BUFF_SIZE, 0);
                if (snd == ERROR)
                {
                    perror("CL_Send_err");
                    exit(EXIT_FAILURE);
                }
                    
	            close(socket_desc);
                print_menu();
                break; 

            case 3:
                printf("Show message delivery status\n");
                break;

            case 4:
                printf("\nJoin the group\n");
                printf("Choose group:\n");
                printf("1 - Alpha\n");
                printf("2 - Beta\n");
                printf("3 - Omega\n");
                
                scanf("%i", &group_choose);
                
                if (group_choose == 1)
                {
                    strcat(group, "Alpha");
                }

                else if (group_choose == 2)
                {
                    strcat(group, "Beta");
                }

                else if (group_choose == 3)
                {
                    strcat(group, "Omega");
                }
                
                else
                {
                    printf("Incorrect number!\nBack to menu\n<-\n");
                    print_menu();
                    break;
                }
                printf("\nChoose group \"%s\"\n", group);
                print_menu();
                break;

            case 5: 
                printf("\nLeave the group\n");
                
                if (strcmp(group, "") == 0)
                {
                    printf("Not in group now\n");
                    print_menu();
                    break;
                }
                
                printf("Leaved from \"%s\"\n", group);         
                print_menu();
                break;
            
            case 6:
                return 0;
        }        
    }
}
