#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../constants.h"

extern void setup_client(char *, u_short);
extern int control_requests();
extern void terminate_client();

int main(int argc, char *argv[]) {
  u_short port = DEF_PORT;
  char server_name[MAX_LEN_NAME];

  sprintf(server_name, "localhost");
  // get server name and port number 
  switch (argc) {
  case 1:
    break;
  case 2:
    sprintf(server_name, "%s", argv[1]);
    break;
  case 3:
    sprintf(server_name, "%s", argv[1]);
    port = (u_short)atoi(argv[2]);
    break;
  default:
    fprintf(stderr, "Usage: %s [server name] [port number]\n", argv[0]);
    return 1;
  }
  // setup client
  setup_client(server_name, port);

  // control requests
  int  cond = 1;
  while (cond) {
    cond = control_requests();
  }

  // terminate client
  terminate_client();

  return 0;
}
