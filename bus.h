#ifndef BUS_H
#define BUS_H

#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

// Bus model supports multiple initiators and multiple targets with b_transport

template <unsigned int N_INITIATORS, unsigned int N_TARGETS>
struct Bus : sc_module
{
  // Tagged sockets allow incoming transactions to be identified
  tlm_utils::simple_target_socket_tagged<Bus> *targ_socket[N_INITIATORS];
  tlm_utils::simple_initiator_socket_tagged<Bus> *init_socket[N_TARGETS];

  SC_CTOR(Bus)
  {
    for (unsigned int i = 0; i < N_INITIATORS; i++)
    {
      char txt[20];
      sprintf(txt, "targ_socket_%d", i);
      targ_socket[i] = new tlm_utils::simple_target_socket_tagged<Bus>(txt);

      targ_socket[i]->register_b_transport(this, &Bus::b_transport, i);
    }
    for (unsigned int i = 0; i < N_TARGETS; i++)
    {
      char txt[20];
      sprintf(txt, "init_socket_%d", i);
      init_socket[i] = new tlm_utils::simple_initiator_socket_tagged<Bus>(txt);
    }
  }

  // Tagged TLM-2 blocking transport method
  virtual void b_transport(int id, tlm::tlm_generic_payload &trans, sc_time &delay)
  {
    if (id < N_INITIATORS)
    {
      // Forward path
      sc_dt::uint64 address = trans.get_address();
      sc_dt::uint64 masked_address;
      unsigned int target_nr = decode_address(address, masked_address);

      if (target_nr < N_TARGETS)
      {
        // Modify address within transaction
        trans.set_address(masked_address);

        // Forward transaction to appropriate target
        (*init_socket[target_nr])->b_transport(trans, delay);

        // Replace original address
        trans.set_address(address);
      }
    }
    else
      SC_REPORT_FATAL("TLM-2", "Invalid tagged socket id in bus");
  }

  // Simple fixed address decoding
  inline unsigned int decode_address(sc_dt::uint64 address, sc_dt::uint64 &masked_address)
  {
    unsigned int target_nr = 0;
    masked_address = address;
    return target_nr;
  }
};

#endif
