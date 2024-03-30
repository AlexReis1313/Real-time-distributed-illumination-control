#include "includes/CanManager.hpp"

CanManager *CanManager::instance = nullptr; // Initialize static instance pointer
static std::map<int, eventFunction> _actionMap;

CanManager::CanManager() : \
    cancanController(spi0, 17, 19, 16, 18, 10000000), dataAvailable(false) {
    instance = this; // Set the instance pointer for ISR access
}

void CanManager::flashIDsetup() {
    rp2040.idleOtherCore();
    flash_get_unique_id(canbus_vars.this_pico_flash_id);
    rp2040.resumeOtherCore();
    canbus_vars.node_address = this_pico_flash_id[7];
}

void CanManager::begin(long bitrate) {
    // Initialize CAN controller
    canController.reset();
    canController.setBitrate(bitrate);
    canController.setNormalMode();
    
    // Initialize interrupt pin for CAN-BUS messages
    pinMode(canbus_vars.interruptPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(canbus_vars.interruptPin), ISR_wrapper, FALLING);
}

void CanManager::canInterrupt() {
    dataAvailable = true; // Set flag when CAN data is available
}

void CanManager::sendMessage1to0(can_bus *frame, can_bus **ptr_frame) {    
    /* Two buffers (RXB0 and RXB1) */
    uint8_t irq = can0.getInterrupts(); // Get the interrupt flags
    if (irq & MCP2515::CANINTF_RX0IF) { // Check if interrupt is from RXB0
        can0.readMessage(MCP2515::RXB0, &(*frame));
        rp2040.fifo.push_nb(&(*frame));
    }
    if (irq & MCP2515::CANINTF_RX1IF) { // Check if interrupt is from RXB1
        can0.readMessage(MCP2515::RXB1, &(*frame));
        rp2040.fifo.push_nb(&(*frame));
    }
    if(rp2040.fifo.pop_nb((*ptr_frame))) { //message from 0 to 1 
        can0.sendMessage((*ptr_frame));
        delete (*ptr_frame);
    }
}

void CanManager::setUpFiltersAndMasks () {
    /* If the received message's identifier matches the filter, it is accepted and processed further.
      A mask is a bit pattern that determines which bits in the filter are significant for comparison */

    canBus.setFilterMask(MCP2515::MASK0, 0, 0x000000ff); //last byte is the significant one
    canBus.setFilter(MCP2515::RXF0, 0, 0); //accept all messages

    canBus.setFilterMask(MCP2515::MASK1, 0, 0x000000ff);
    canBus.setFilter(MCP2515::RXF1, 0, canbus_vars.node_address);
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

// //Unpacks an unsigned int into its constituting 4 bytes
// void msg_to_bytes(uint32_t msg, uint8_t * bytes) {
//     bytes[0] = msg; bytes[1] = (msg >> 8);
//     bytes[2] = (msg >> 16); bytes[3] = (msg >> 24);
// }
// //Packs the CAN frame contents into an unsigned int
// uint32_t can_frame_to_msg(can_frame * frm) {
//     uint8_t b[4];
//     b[3] = ReadData; b[2] = frm->can_id;
//     b[1] = frm->data[1]; b[0] = frm->data[0];
//     return bytes_to_msg(b);
// }
// //Packs the CAN error flags into an unsigned int
// uint32_t error_flags_to_msg(uint8_t canintf, uint8_t eflg) {
//     uint8_t b[4];
//     b[3] = ErrorData; b[2] = 0;
//     b[1] = canintf; b[0] = eflg;
//     return bytes_to_msg(b);
// }