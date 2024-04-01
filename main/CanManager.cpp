#include "includes/CanManager.hpp"

CanManager *CanManager::instance = nullptr; // Initialize static instance pointer
std::map<int, eventFunction> CanManager::_actionMap;
canBus_vars CanManager::canbus_vars;
MCP2515 CanManager::canController(spi0, 17, 19, 16, 18, 10000000);
volatile bool CanManager::dataAvailable = false;
unsigned char CanManager::hub = 0;
bool CanManager::hubFlag = false;
bool CanManager::hubFound = false;

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
    //pinMode(canbus_vars.interruptPin, INPUT_PULLUP);
    //attachInterrupt(digitalPinToInterrupt(canbus_vars.interruptPin), ISR_wrapper, FALLING);
}

void CanManager::printID() {
    Serial.print("Node address: ");
    Serial.println(canbus_vars.node_address);
}

void CanManager::canInterrupt() {
    dataAvailable = true; // Set flag when CAN data is available
}

void CanManager::canBusRotine() {    
    /* Two buffers (RXB0 and RXB1) */
    can_frame frame;
    uint32_t framePtrVal;
    unsigned char irq = canController.getInterrupts(); // Get the interrupt flags
    if (irq & MCP2515::CANINTF_RX0IF) { // Check if interrupt is from RXB0
        canController.readMessage(MCP2515::RXB0, &frame);
        framePtrVal = reinterpret_cast<uint32_t>(&frame);
        rp2040.fifo.push_nb(framePtrVal);
    }
    if (irq & MCP2515::CANINTF_RX1IF) { // Check if interrupt is from RXB1
        canController.readMessage(MCP2515::RXB1, &frame);
        framePtrVal = reinterpret_cast<uint32_t>(&frame);
        rp2040.fifo.push_nb(framePtrVal);
    }
    uint32_t poppedFrameAddress;
    if(rp2040.fifo.pop_nb(&poppedFrameAddress)) { 
        can_frame* poppedFrame = reinterpret_cast<can_frame*>(poppedFrameAddress);
        canController.sendMessage(poppedFrame);
        delete (poppedFrame);
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

//Falta sistema de identificação do sender
//type DONE

//          1       2       3      4      5      6       7       8     (bytes) // 8*8 bits = 64bits
// DATA | sender | type |  INT    INT    INT    INT  |       |       |
void CanManager::enqueue_message(unsigned char sender, my_type type, unsigned char *message, std::size_t msg_size)
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

info_msg CanManager::extract_message(can_frame* frame) {
  info_msg result;
  memset(&result, 0, sizeof(result));
  result.size = static_cast<size_t>(frame->can_dlc - 2);
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

void CanManager::serial_and_actions_rotine(void) {
    can_frame *frame;

    while (rp2040.fifo.available()) {
        if (!rp2040.fifo.pop_nb((uint32_t *)&frame)) 
            break;
        info_msg pm = CanManager::extract_message(frame);
        my_type message_type = pm.type;
        unsigned char* data = pm.data;
        
        std::map<int, eventFunction>::iterator it = _actionMap.find(message_type);
        if (_actionMap.find(message_type) != _actionMap.end()){
            it->second(pm);
            Serial.print("Type: "); Serial.print(it->first);
        }
    }
    if (Serial.available()) 
    {
        if (!(CanManager::hubFound)){ //if no hub has yet been found
        CanManager::checkHub();         //define this pico as hub and warn other picos to stop looking for hubs
        }
        if (CanManager::hubFlag){ //if I am HUB
            String command = Serial.readStringUntil('\n');
            command.trim(); // Remove any whitespace
            my()->my_parser.parseCommand(command); //execute serial message
        }

    }
}

void CanManager::checkHub() {
    if (Serial.available() > 0) {
        CanManager::hubFlag = true;
        CanManager::hubFound = true;
        unsigned char data[sizeof(bool)];
        memcpy(data, &CanManager::hubFound, sizeof(bool));
        HUB = PICO_ID;
        CanManager::enqueue_message(PICO_ID, my_type::FOUND_HUB, data, sizeof(bool));
    }
}


// void CanManager::wake_up_grid() {
//         //initialize list_IDs with first array value equal to PICO_ID
//         nr_ckechIn_Node = 1
//         while(!(CanManager::check_wake_up_condition())){
        
//             my()->current_time = millis();
//             if (my()->current_time - my()->last_control_time >= my()->control_interval) {
//                 unsigned char *data;
//                 memcpy(&data, nr_ckechIn_Node, sizeof(int)));
//                 CanManager::enqueue_message(PICO_ID, my_type::WakeUp, data, sizeof(PICO_ID));
//                 my()->last_control_time = my()->current_time;

//         }

//         sort (list_IDS)
//         for i in range list_nodes
//         list_nodes.append(i)

// }

// bool CanManager::check_wake_up_condition(){
//     if (all values in list_Nr_detected_IDS are equal to nr_ckechIn_Nodes && nr_ckechIn_Nodes>1 )
//     return true
//     else 
//     return false

// }


// void CanManager::WakeUpAction(info_msg &msg) {
    
// if (msg.sender is different from all values in list_IDs){
//     list_IDs.append(msg.sender) 
//     list_Nr_detected_IDS.append(msg.data)
    
//     nr_ckechIn_Nodes =length(list_IDs) + 1


// }

       

// }

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