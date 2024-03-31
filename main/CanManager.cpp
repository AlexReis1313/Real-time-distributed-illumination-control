#include "includes/CanManager.hpp"

CanManager *CanManager::instance = nullptr; // Initialize static instance pointer
static std::map<int, eventFunction> _actionMap;

CanManager::CanManager() : \
    canController(spi0, 17, 19, 16, 18, 10000000), dataAvailable(false) {
    instance = this; // Set the instance pointer for ISR access
}

void CanManager::flashIDsetup() {
    rp2040.idleOtherCore();
    flash_get_unique_id(canbus_vars.this_pico_flash_id);
    rp2040.resumeOtherCore();
    canbus_vars.node_address = canbus_vars.this_pico_flash_id[7];
}

void CanManager::begin(char bitrate) {
    // Initialize CAN controller
    canController.reset();
    canController.setNormalMode();
    
    // Initialize interrupt pin for CAN-BUS messages
    pinMode(canbus_vars.interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(canbus_vars.interruptPin), ISR_wrapper, FALLING);
}

void CanManager::canInterrupt() {
    dataAvailable = true; // Set flag when CAN data is available
}

void CanManager::sendMessage1to0() {    
    /* Two buffers (RXB0 and RXB1) */
    can_frame frame, *ptr_frame;
    unsigned char irq = canController.getInterrupts(); // Get the interrupt flags
    if (irq & MCP2515::CANINTF_RX0IF) { // Check if interrupt is from RXB0
        canController.readMessage(MCP2515::RXB0, frame);
        rp2040.fifo.push_nb(*frame);
    }
    if (irq & MCP2515::CANINTF_RX1IF) { // Check if interrupt is from RXB1
        canController.readMessage(MCP2515::RXB1, frame);
        rp2040.fifo.push_nb(*frame);
    }
    if(rp2040.fifo.pop_nb((*ptr_frame))) { //message from 0 to 1 
        canController.sendMessage((*ptr_frame));
        delete (*ptr_frame);
    }
}

void CanManager::setUpFiltersAndMasks () {
    /* If the received message's identifier matches the filter, it is accepted and processed further.
      A mask is a bit pattern that determines which bits in the filter are significant for comparison */

    canController.setFilterMask(MCP2515::MASK0, 0, 0x000000ff); //last byte is the significant one
    canController.setFilter(MCP2515::RXF0, 0, 0); //accept all messages

    canController.setFilterMask(MCP2515::MASK1, 0, 0x000000ff);
    canController.setFilter(MCP2515::RXF1, 0, canbus_vars.node_address);
}

bool CanManager::receiveMessage(struct can_frame &msg) {
    if (dataAvailable && canController.readMessage(&msg) == MCP2515::ERROR_OK) {
        dataAvailable = false; // Reset the flag
        return true;
    }
    return false;
}

bool CanManager::data_available() {
    return dataAvailable;
}

// 8*8 bits = 64bits
//          1       2       3      4      5      6       7       8     (bytes)
// DATA | sender | type |  INT    INT    INT    INT  |       |       |
void CanManager::enqueue_message(unsigned char* sender, my_type type, unsigned char* *message, std::size_t msg_size)
{
  can_frame *new_frame = new can_frame;
  std::size_t length = min(msg_size + 2, CAN_MAX_DLEN);
  new_frame->can_id = sender; //11 bits
  new_frame->can_dlc = length; //nb of bytes available (saved in 4bits)
  if (message && msg_size > 0)
    memcpy(new_frame->data + 2, message, length - 2);
  if (message && msg_size > length - 2)
    new_frame->can_id |= ((uint32_t) message[length - 2]) << 8;  
  if (message && msg_size > length - 1)
    new_frame->can_id |= ((uint32_t) message[length - 1]) << 16;    
  new_frame->data[0] = canbus_vars.node_address;
  new_frame->data[1] = type;
  //tell the other the action done
  rp2040.fifo.push_nb((uint32_t)new_frame);
}

static info_msg CanManager::extract_message(can_frame* frame) {
  info_msg result;
  memset(&result, 0, sizeof(result));
  result.size = frame->can_dlc - 2;  
  result.type = (my_type)frame->data[1];
  result.sender = frame->data[0];
  memcpy(result.data, &frame->data[2], result.size);

  //todo
  if (frame->can_dlc > 6)
    result.data[6] = (frame->can_id & 0x0000ff00) >> 8;
  if (frame->can_dlc > 7)
    result.data[7] = (frame->can_id & 0x00ff0000) >> 16;
  return result;
}


// static void CanManager::enqueue_message(unsigned char* sender, my_type type, unsigned char* *message, std::size_t msg_size) {
//     can_frame frame;
//     frame.can_id = canbus_vars.node_address;
//     frame.can_dlc = msg_size + 2;
//     frame.data[0] = sender;
//     frame.data[1] = type;
//     memcpy(&frame.data[2], message, msg_size);
//     canController.sendMessage(&frame);
// }

// //Unpacks an unsigned int into its constituting 4 bytes
// void msg_to_bytes(uint32_t msg, unsigned char* * bytes) {
//     bytes[0] = msg; bytes[1] = (msg >> 8);
//     bytes[2] = (msg >> 16); bytes[3] = (msg >> 24);
// }
// //Packs the CAN frame contents into an unsigned int
// uint32_t can_frame_to_msg(can_frame * frm) {
//     unsigned char* b[4];
//     b[3] = ReadData; b[2] = frm->can_id;
//     b[1] = frm->data[1]; b[0] = frm->data[0];
//     return bytes_to_msg(b);
// }
// //Packs the CAN error flags into an unsigned int
// uint32_t error_flags_to_msg(unsigned char* canintf, unsigned char* eflg) {
//     unsigned char* b[4];
//     b[3] = ErrorData; b[2] = 0;
//     b[1] = canintf; b[0] = eflg;
//     return bytes_to_msg(b);
// }