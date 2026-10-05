// program3
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#include "../constants.h"

// global variables
static int n_clients;
static int my_id;
static int sock;
static int num_sock;
static fd_set mask;
static CLIENT clients[MAX_NUM_CLIENTS];

void setup_client(char *, u_short);
int control_requests();
void terminate_client();

static int in_command(void);
static int exe_command(void);
static void send_data(void *, int);
static int receive_data(void *, int);
static void handle_error(char *);

// set up the client side and connect to the server
void setup_client(char *server_name, u_short port) {
  struct hostent *server;
  struct sockaddr_in sv_addr;
  // initialize the client socket and connect to the server
  fprintf(stderr, "Trying to connect server %s (port = %d).\n", server_name, port);
  if ((server = gethostbyname(server_name)) == NULL) {
    handle_error("gethostbyname()");
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    handle_error("socket()");
  }

  sv_addr.sin_family = AF_INET;
  sv_addr.sin_port = htons(port);
  sv_addr.sin_addr.s_addr = *(u_int *)server->h_addr_list[0];
  // connect to the server and handle any errors that may occur
  if (connect(sock, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) != 0) {
    handle_error("connect()");
  }
  // prompt the user to input their name and send it to the server
  fprintf(stderr, "Input your name: ");
  char user_name[MAX_LEN_NAME];
  if (fgets(user_name, sizeof(user_name), stdin) == NULL) {
    handle_error("fgets()");
  }
  user_name[strlen(user_name) - 1] = '\0';
  send_data(user_name, MAX_LEN_NAME);

  fprintf(stderr, "Waiting for other clients...\n");
  receive_data(&n_clients, sizeof(int));
  fprintf(stderr, "Number of clients = %d.\n", n_clients);
  receive_data(&my_id, sizeof(int));
  fprintf(stderr, "Your ID = %d.\n", my_id);
  int i;
  for (i = 0; i < n_clients; i++) {
    receive_data(&clients[i], sizeof(CLIENT));
  }

  num_sock = sock + 1;
  FD_ZERO(&mask);
  FD_SET(0, &mask);
  FD_SET(sock, &mask);
  fprintf(stderr, "Input command (M=message, Q=quit): \n");
}

// function to control requests from the user and the server
int control_requests () {
  fd_set read_flag = mask;

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 30;

  if (select(num_sock, (fd_set *)&read_flag, NULL, NULL, &timeout) == -1) {
    handle_error("select()");
  }

  int result = 1;
  if (FD_ISSET(0, &read_flag)) {
     result = in_command();
  } else if (FD_ISSET(sock, &read_flag)) {
    result = exe_command();
  }

  return result;
}

// function to handle user input commands
static int in_command() {
  CONTAINER data;
  char com;
  memset(&data, 0, sizeof(CONTAINER));
  com = getchar();
  while(getchar()!='\n');
  // execute the command based on user input
  switch (com) {
  case MESSAGE_COMMAND:
    // prompt the user to input a message and send it to the server
    fprintf(stderr, "Input message: ");
    if (fgets(data.message, MAX_LEN_MESSAGE, stdin) == NULL) {
      handle_error("fgets()");
    }
    data.command = MESSAGE_COMMAND;
    data.message[strlen(data.message)-1] = '\0';
    data.cid = my_id;
    // send the message to the server
    send_data(&data, sizeof(CONTAINER));
    break;
  case QUIT_COMMAND:
    // send a quit command to the server and terminate the client
    data.command = QUIT_COMMAND;
    data.cid = my_id;
    // send the quit command to the server
    send_data(&data, sizeof(CONTAINER));
    break;
  default:
    // handles unexpected commands
    fprintf(stderr, "%c is not a valid command.\n", com);
  }

  return 1;
}

// function to execute commands received from the server
static int exe_command() {
  CONTAINER data;
  int result = 1;
  memset(&data, 0, sizeof(CONTAINER));
  // receive data from the server
  receive_data(&data, sizeof(data));
  // execute the command based on the received data
  switch (data.command) {
  case MESSAGE_COMMAND:
    // display the message received from the server
    fprintf(stderr, "client[%d] %s: %s\n", data.cid, clients[data.cid].name, data.message);
    result = 1;
    break;
  case QUIT_COMMAND:
    // display a message indicating that the client has sent a quit command
    fprintf(stderr, "client[%d] %s sent quit command.\n", data.cid, clients[data.cid].name);
    result = 0;
    break;
  case TURN_COMMAND:
    // check if it is the client's turn to send a message
    if (data.cid == my_id) {
      fprintf(stderr, "It is your turn.\n");
    } else {
      fprintf(stderr, "Waiting for client[%d] %s.\n",
              data.cid, clients[data.cid].name);
    }
    result = 1;
    break;
  case WARNING_COMMAND:
    // display a warning message if it is not the client's turn to send a message
    fprintf(stderr, "Warning: %s\n", data.message);
    result = 1;
    break;
  default:
    // handles unexpected commands
    fprintf(stderr, "exe_command(): %c is not a valid command.\n", data.command);
    exit(1);
  }

  return result;
}
// function to send data to the server
static void send_data(void *data, int size) {
  if ((data == NULL) || (size <= 0)) {
    fprintf(stderr, "send_data(): data is illeagal.\n");
    exit(1);
  }
  // send the data to the server and handle any errors that may occur
  if (write(sock, data, size) == -1) {
    handle_error("write()");
  }
}
// function to receive data from the server
static int receive_data(void *data, int size) {
  if ((data == NULL) || (size <= 0)) {
    fprintf(stderr, "receive_data(): data is illeagal.\n");
    exit(1);
  }
  // receive the data from the server and handle any errors that may occur
  return(read(sock, data, size));
}
// function to handle errors and print error messages
static void handle_error(char *message) {
  perror(message);
  fprintf(stderr, "%d\n", errno);
  exit(1);
}
// function to terminate the client and close the connection to the server
void terminate_client() {
  fprintf(stderr, "Connection is closed.\n");
  close(sock);
  exit(0);
}
