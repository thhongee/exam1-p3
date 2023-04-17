#ifndef TARGET_H
#define TARGET_H

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

#include "systemc"
using namespace sc_core;
using namespace sc_dt;
using namespace std;

#include "tlm.h"
#include "tlm_utils/simple_target_socket.h"
#include <cstdint>
#include "map.h"

// TLM target decimation function
SC_MODULE(Decimation)
{
  // TLM-2 socket, base protocol
  tlm_utils::simple_target_socket<Decimation> socket;

  enum
  {
    SIZE = 3
  };

  SC_CTOR(Decimation)
      : socket("socket")
  {
    // Register callback for incoming b_transport interface method call
    socket.register_b_transport(this, &Decimation::b_transport);

    // Initialize input and output data register
    for (int i = 0; i < SIZE; i++)
      i_data[i] = 0;
    sum = 0;
  }

  // TLM-2 blocking transport method
  virtual void b_transport(tlm::tlm_generic_payload & trans, sc_time & delay)
  {
    tlm::tlm_command cmd = trans.get_command();
    sc_dt::uint64 adr = trans.get_address();
    unsigned char *ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();
    unsigned char *byt = trans.get_byte_enable_ptr();
    unsigned int wid = trans.get_streaming_width();

    // Obliged to implement read and write commands
    if (cmd == tlm::TLM_READ_COMMAND)
    {
      if (adr == BASE_TARGET_OUTPUT_ADDR)
      {
        // Copy 3 bytes to ptr
        for (unsigned int i = 0; i < len; i++)
          ptr[i] = sum.range(3 + (i << 2), i << 2);
        delay = sc_time(5, SC_NS); // Accumulate delay for quantum keeper
      }
      else
      {
        SC_REPORT_ERROR("TLM-2", "Address not supported for read operation.");
      }
    }
    else if (cmd == tlm::TLM_WRITE_COMMAND)
    {
      if (adr == BASE_TARGET_INPUT_ADDR)
      {
        // Copy 3 bytes from ptr
        for (unsigned int i = 0; i < len; i++) // 複製3個data進來計算
          i_data[i] = ptr[i];
        // Compute summation with lower two uint8_t integers
        sum = i_data[0] * (1 / 6) + i_data[1] * (1 / 3) + i_data[2] * (1 / 2); // 把三個值依序乘上h[k]
        delay = sc_time(10, SC_NS);                                            // Accumulate delay for quantum keeper
      }
      else
      {
        SC_REPORT_ERROR("TLM-2", "Address not supported for write operation.");
      }
    }

    // Obliged to set response status to indicate successful completion
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
  }

  uint8_t i_data[SIZE];
  sc_int<32> sum;
};

#endif
