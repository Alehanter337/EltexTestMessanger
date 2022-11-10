#define ERROR -1
#define NO_ARGS 1
#define EQUAL 0
#define DEL_CLIENTS_LEN 8
#define BUFF_SIZE 2048
#define MAX_USER_LEN 32
#define MAX_USERF_LEN 64
#define MAX_ADDR_LEN 16
#define MAX_INBOX_LEN 2048 
#define RED "\033[31m"
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

void left_to_var(char buff[BUFF_SIZE], char destination[MAX_USER_LEN]);
