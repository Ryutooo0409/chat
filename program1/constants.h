#include <netinet/in.h>

// define max measurements
#define DEF_PORT 52026
#define MAX_LEN_NAME 100
#define MAX_NUM_CLIENTS 5
#define MAX_LEN_MESSAGE 256
#define MAX_LEN_ADDR 32
#define BROADCAST -1

// define the types of input commands
#define MESSAGE_COMMAND 'M'
#define QUIT_COMMAND 'Q'

// client side structure
typedef struct {
  int cid;
  int sock;
  struct sockaddr_in addr;
  char name[MAX_LEN_NAME];
} CLIENT;

// container structure
typedef struct {
  int cid;
  char command;
  char message[MAX_LEN_MESSAGE];
} CONTAINER;
