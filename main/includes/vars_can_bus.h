#include <hardware/flash.h>
#include "mcp2515.h"
#include <map>

// Unique identifiers for inter-core communication commands
enum InterCoreCmd {
    ReadData =  1,   // From core 1 to core 0: contains data read (16 bit)
    WriteData = 2,   // From core 0 to core 1: contains data to write (16 bit)
    ErrorData = 3    // From core 1 to core 0: contains regs CANINTF, EFLG
};

// Can-bus setup
struct canBus_vars {
    unsigned char this_pico_flash_id[8];
    unsigned char node_address;  // Node address (last byte of the flash ID)
    struct can_frame canMsgTx;
    struct can_frame canMsgRx;
    unsigned long counterTx;
    unsigned long counterRx;
    MCP2515::ERROR err;
    unsigned long time_to_write;
    unsigned long write_delay;
    const byte interruptPin;

    canBus_vars() : counterTx(0), counterRx(0), write_delay(1000), interruptPin(20) {}
};

enum my_type : uint8_t //needs to 8 bits
{
  OFF,
  ON,
  ACK,
  SET_DUTY_CYCLE,
  GET_DUTY_CYCLE,
  SET_REFERENCE,
  GET_REFERENCE, 
  SERIAL_GET_REFERENCE,
};

struct info_msg {
  my_type type;
  size_t size;
  unsigned char sender, data[8];
};