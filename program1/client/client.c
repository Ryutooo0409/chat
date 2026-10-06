// program1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

// importing constants 
#include "../constants.h"

// declaring number of clients, id , sock variable etc.
static int n_clients;
static int my_id;
static int sock;
static int num_sock;
static fd_set mask;
// declares the all clients with max number of clients
static CLIENT clients[MAX_NUM_CLIENTS];

// declares the functions such as set up client, control requests, delete client
void setup_client(char *, u_short);
int control_requests();
void terminate_client();

// basic functions for sending and receiving data 
static int in_command(void);
static int exe_command(void);
static void send_data(void *, int);
static int receive_data(void *, int);
static void handle_error(char *);

// set up client  side function
void setup_client(char *server_name, u_short port) {
  struct hostent *server;
  struct sockaddr_in sv_addr;

  // try to connect to the server with server name and port number. Also handles the error with unknown server
  fprintf(stderr, "Trying to connect server %s (port = %d).\n", server_name, port);
  if ((server = gethostbyname(server_name)) == NULL) {
    handle_error("gethostbyname()");
  }
  // creates a sock and handles the error with uncreated sock
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    handle_error("socket()");
  }
   
  sv_addr.sin_family = AF_INET;
  sv_addr.sin_port = htons(port);
  sv_addr.sin_addr.s_addr = *(u_int *)server->h_addr_list[0];

  // try to connect to sock and handles the error with it 
  if (connect(sock, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) != 0) {
    handle_error("connect()");
  }

  // gets a client name and handles the error with null user_name 
  fprintf(stderr, "Input your name: ");
  char user_name[MAX_LEN_NAME];
  if (fgets(user_name, sizeof(user_name), stdin) == NULL) {
    handle_error("fgets()");
  }
  // sends the user name to the server 
  user_name[strlen(user_name) - 1] = '\0';
  send_data(user_name, MAX_LEN_NAME);

  // waits until receive number of clients
  fprintf(stderr, "Waiting for other clients...\n");
  receive_data(&n_clients, sizeof(int));
  // if it receives the number of clients it prints in clients terminal
  fprintf(stderr, "Number of clients = %d.\n", n_clients);
  // Also the clients receive their own id and terminal prints it
  receive_data(&my_id, sizeof(int));
  fprintf(stderr, "Your ID = %d.\n", my_id);
  int i;
  // receive all clients address of variable
  for (i = 0; i < n_clients; i++) {
    receive_data(&clients[i], sizeof(CLIENT));
  }
  // initialize the getting commands
  num_sock = sock + 1;
  FD_ZERO(&mask);
  FD_SET(0, &mask);
  FD_SET(sock, &mask);
  fprintf(stderr, "Input command (M=message, Q=quit): \n");
}

int control_requests () {
  fd_set read_flag = mask;

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 30;
  // Handles the error with client side select functionality
  if (select(num_sock, (fd_set *)&read_flag, NULL, NULL, &timeout) == -1) {
    handle_error("select()");
  }

  // sets result true if the communication continues
  int result = 1;
  if (FD_ISSET(0, &read_flag)) {
     result = in_command();
  } else if (FD_ISSET(sock, &read_flag)) {
    result = exe_command();
  }

  return result;
}

// handles command input types (message or quit) 
static int in_command() {
  // creates temporary variables for mainly message data
  CONTAINER data;
  char com;
  memset(&data, 0, sizeof(CONTAINER));
  // gets a command with M(essage) or Q(uit)
  com = getchar();
  // loops until input something 
  while(getchar()!='\n');

  // handles the types of commands 
  switch (com) {
  case MESSAGE_COMMAND: 
    // if message command is M, it sends a data  
    fprintf(stderr, "Input message: ");
    if (fgets(data.message, MAX_LEN_MESSAGE, stdin) == NULL) {
      handle_error("fgets()");
    }
    data.command = MESSAGE_COMMAND;
    data.message[strlen(data.message)-1] = '\0';
    data.cid = my_id;
    send_data(&data, sizeof(CONTAINER));
    break;
  case QUIT_COMMAND:
    // if message command is Q, it sends a data with quit command
    data.command = QUIT_COMMAND;
    data.cid = my_id;
    send_data(&data, sizeof(CONTAINER));
    break;
  default:
    // handles an other invalid input  
    fprintf(stderr, "%c is not a valid command.\n", com);
  }

  return 1;
}

// executes the commands  
static int exe_command() {
  CONTAINER data;
  int result = 1;
  memset(&data, 0, sizeof(CONTAINER));
  // receive a data
  receive_data(&data, sizeof(data));

  // switch through data command
  switch (data.command) {
  case MESSAGE_COMMAND:
    // if it is message command, prints out metadata of the data and return true to continue the communication 
    fprintf(stderr, "client[%d] %s: %s\n", data.cid, clients[data.cid].name, data.message);
    result = 1;
    break;
  case QUIT_COMMAND:
    // if it is quit command, prints out metadata of the data and return false to stop the communication
    fprintf(stderr, "client[%d] %s sent quit command.\n", data.cid, clients[data.cid].name);
    result = 0;
    break;
  default:
    // checks if it is an invalid command it handles it and exit this function
    fprintf(stderr, "exe_command(): %c is not a valid command.\n", data.command);
    exit(1);
  }
  // return the result in default condition
  return result;
}

// data sending function 
static void send_data(void *data, int size) {
  // if a data is null or size is less than zero it handles the error
  if ((data == NULL) || (size <= 0)) {
    fprintf(stderr, "send_data(): data is illeagal.\n");
    exit(1);
  }
  // if a data is available, write the data to sock 
  if (write(sock, data, size) == -1) {
    handle_error("write()");
  }
}

// data receiving function
static int receive_data(void *data, int size) {
  // if a data is null or size is less than zero it handles the error
  if ((data == NULL) || (size <= 0)) {
    fprintf(stderr, "receive_data(): data is illeagal.\n");
    exit(1);
  }
  // then gets data 
  return(read(sock, data, size));
}

// error handling function
static void handle_error(char *message) {
  perror(message);
  fprintf(stderr, "%d\n", errno);
  exit(1);
}
// disconnect from the server and close the sock
void terminate_client() {
  fprintf(stderr, "Connection is closed.\n");
  close(sock);
  exit(0);
}
