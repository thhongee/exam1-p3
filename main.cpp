#include "initiator.h"
#include "target.h"
#include "bus.h"

int sc_main(int argc, char *argv[])
{
  Initiator initiator("initiator");
  Decimation decimation("decimation");
  Bus<1, 1> bus("bus");

  // Bind initiator socket to target socket
  initiator.socket.bind(*(bus.targ_socket[0]));
  (*(bus.init_socket[0])).bind(decimation.socket);

  sc_start();
  return 0;
}
