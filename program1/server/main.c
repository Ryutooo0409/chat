#include <stdio.h>
#include <stdlib.h>

#include "../constants.h"

extern void setup_server(int, u_short);
extern int control_requests();
extern void terminate_server();

int main(int argc, char *argv[]) {
  // default number of clients sets to 1
  int num_cl = 1;
  u_short port = DEF_PORT;
  
  // gets number of clients and port number 
  switch (argc) {
    case 1:
      break;
    case 2:
      num_cl = atoi(argv[1]);
      break;
    case 3:
      num_cl = atoi(argv[1]);
      port = atoi(argv[2]);
      break;
    default:
      fprintf(stderr, "Usage: %s [number of clients] [port number]\n", argv[0]);
      return 1;
  }

  // checks if number of clients exceeds maximum number of clients
  if (num_cl < 0 || num_cl > MAX_NUM_CLIENTS) {
    fprintf(stderr, "Max number of clients is %d\n", MAX_NUM_CLIENTS);
    return 1;
  }
  
  // prints number of clients and port number
  fprintf(stderr, "Number of clients = %d\n", num_cl);
  fprintf(stderr, "Port number = %d\n", port);

  // setups the server with number of clients and port number
  setup_server(num_cl, port);

  // loops through control requests
  int cond = 1;
  while (cond) {
      cond = control_requests();
  }

  // terminates the server
  terminate_server();

  return 0;
}
