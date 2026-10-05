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

#include "../constants.h"

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

void setup_client(char *server_name, u_short port) {
  struct hostent *server;
  struct sockaddr_in sv_addr;

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

  if (connect(sock, (struct sockaddr *)&sv_addr, sizeof(sv_addr)) != 0) {
    handle_error("connect()");
  }

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

static int in_command() {
  CONTAINER data;
  char com;
  memset(&data, 0, sizeof(CONTAINER));
  com = getchar();
  while(getchar()!='\n');

  switch (com) {
  case MESSAGE_COMMAND:
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
    data.command = QUIT_COMMAND;
    data.cid = my_id;
    send_data(&data, sizeof(CONTAINER));
    break;
  default:
    fprintf(stderr, "%c is not a valid command.\n", com);
  }

  return 1;
}

static int exe_command() {
  CONTAINER data;
  int result = 1;
  memset(&data, 0, sizeof(CONTAINER));
  receive_data(&data, sizeof(data));

  switch (data.command) {
  case MESSAGE_COMMAND:
    fprintf(stderr, "client[%d] %s: %s\n", data.cid, clients[data.cid].name, data.message);
    result = 1;
    break;
  case QUIT_COMMAND:
    fprintf(stderr, "client[%d] %s sent quit command.\n", data.cid, clients[data.cid].name);
    result = 0;
    break;
  default:
    fprintf(stderr, "exe_command(): %c is not a valid command.\n", data.command);
    exit(1);
  }

  return result;
}

static void send_data(void *data, int size) {
  if ((data == NULL) || (size <= 0)) {
    fprintf(stderr, "send_data(): data is illeagal.\n");
    exit(1);
  }

  if (write(sock, data, size) == -1) {
    handle_error("write()");
  }
}

static int receive_data(void *data, int size) {
  if ((data == NULL) || (size <= 0)) {
    fprintf(stderr, "receive_data(): data is illeagal.\n");
    exit(1);
  }

  return(read(sock, data, size));
}

static void handle_error(char *message) {
  perror(message);
  fprintf(stderr, "%d\n", errno);
  exit(1);
}

void terminate_client() {
  fprintf(stderr, "Connection is closed.\n");
  close(sock);
  exit(0);
}
