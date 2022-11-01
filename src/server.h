#define ERROR -1
#define NO_ARGS 1
#define EQUAL 0
#define BUFF_SIZE 256
#define MAX_USER_LEN 16
#define MAX_ADDR_LEN 16
#define MAX_INBOX_LEN 1024 

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

void left_to_var(char buff[BUFF_SIZE], char destination[MAX_USER_LEN]);
