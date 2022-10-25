#include <stdio.h>
#include <string.h>

#define NO_ARGS 1
#define MAX_ADDR_LEN 16
#define MAX_USER_LEN 10

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

int main(int argc, char *argv[])  {
    
    char server_address[MAX_ADDR_LEN] = { };
    char username[MAX_USER_LEN] = { }; 
    char group[MAX_USER_LEN] = "";
    int group_choose = 0;
    int action = 0;

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
    
    printf("Hello, %s!\n", username);
    print_menu();

    while(1)
    {
        scanf("%i", &action);
        
        switch(action) 
        {
            case 1:
                printf("check inbox\n");
                break;

            case 2: 
                printf("send message\n");
                break;

            case 3:
                printf("show message delivery status\n");
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
                    printf("Incorrect number!\n");
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
