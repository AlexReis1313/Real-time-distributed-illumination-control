#include "includes/CanManager.hpp"

CanManager *CanManager::instance = nullptr; // Initialize static instance pointer
std::map<my_type, eventFunction> CanManager::_actionMap;
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
    canbus_vars.node_address = canbus_vars.this_pico_flash_id[6];
}

void CanManager::begin(char bitrate) { // Initialize CAN controller
    canController.reset();
    canController.setNormalMode();
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
    bool message_received = false;
    if (irq & MCP2515::CANINTF_RX0IF) { // Check if interrupt is from RXB0
        //Serial.println("READ FROM RX0IF");
        //Serial.println("READ FROM RX0IF");
        canController.readMessage(MCP2515::RXB0, &frame);
        framePtrVal = reinterpret_cast<uint32_t>(&frame);
        rp2040.fifo.push_nb(framePtrVal);
        message_received = true;
    }
    if (irq & MCP2515::CANINTF_RX1IF) { // Check if interrupt is from RXB1
        //Serial.println("READ FROM RX1IF");
        canController.readMessage(MCP2515::RXB1, &frame);
        framePtrVal = reinterpret_cast<uint32_t>(&frame);
        rp2040.fifo.push_nb(framePtrVal);
        message_received = true;
    }
    if (message_received) {
        canController.clearInterrupts();
    }
    uint32_t poppedFrameAddress;
    if(rp2040.fifo.pop_nb(&poppedFrameAddress)) { 
        //Serial.println("SEND MESSAGE ");
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
  //Serial.println("::Enqueue message::");
  //tell the other the action done
  //Serial.println("------------------------------------------------");
  //Serial.println("::Enqueue message::");
  //Serial.print("can_id: "); Serial.println(new_frame->can_id);
  //Serial.print("node address: "); Serial.println(new_frame->data[0]);
  //Serial.print("type: "); Serial.println(new_frame->data[1]);
  //Serial.println("------------------------------------------------");

  rp2040.fifo.push_nb((uint32_t)new_frame);
}

info_msg CanManager::extract_message(can_frame* frame) {
  info_msg result;
  memset(&result, 0, sizeof(result));
  result.size = static_cast<size_t>(frame->can_dlc - 2);
  result.type = (my_type)frame->data[1];
  result.sender = frame->data[0];
  result.can_id = frame->can_id;
  memcpy(result.data, &frame->data[2], result.size);
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
        std::map<my_type, eventFunction>::iterator it = _actionMap.find(message_type);
        if (it != _actionMap.end()) {
            //Serial.print("Action found for message type: "); Serial.println(static_cast<int>(message_type));
            (*it).second(pm);
        } else {
            Serial.println("No action found for this message type.");
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

void CanManager::canBUS_to_actions_rotine(bool executeAction) {
    can_frame *frame;

    while (rp2040.fifo.available()) {
        if (!rp2040.fifo.pop_nb((uint32_t *)&frame)) 
            break;
        info_msg pm = CanManager::extract_message(frame);
        my_type message_type = pm.type;
        unsigned char* data = pm.data;
        std::map<my_type, eventFunction>::iterator it = _actionMap.find(message_type);
        if (it != _actionMap.end()) {
            //Serial.print("Action found for message type: "); Serial.println(static_cast<int>(message_type));
            if (executeAction){
            (*it).second(pm);
            }
        } else {
            Serial.println("No action found for this message type.");
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
        Serial.print("HUB:: I am the HUB: "); Serial.println(HUB);
        CanManager::enqueue_message(PICO_ID, my_type::FOUND_HUB, data, sizeof(bool));
    }
}



void CanManager::acknoledge(char type,unsigned char data_to_send ){

    
    if (type == 'i'){ //internal ack
        CanManager::enqueue_message(PICO_ID, my_type::ACKINTERNA, &data_to_send, sizeof(data_to_send));
    }
    else if (type =='e'){ //external ack
        CanManager::enqueue_message(PICO_ID, my_type::ACK, &data_to_send, sizeof(data_to_send));
    }
    else if (type =='c'){ //external ack
        //Serial.println("ACK c");
        CanManager::enqueue_message(PICO_ID, my_type::ACKBEGINCONSENSUS, &data_to_send, sizeof(data_to_send));
    }
}
void CanManager::loopUntilACK(int nrOfAcknoledge, unsigned char sender, my_type type, unsigned char *message, std::size_t msg_size){
    bool executeAction=false;
    delay(500);
    CanManager::canBUS_to_actions_rotine(executeAction);//clear buffer of canBUS
    executeAction = true;
    my()->last_control_time = 0;
    my()->list_Ack_loopUntilACK.clear();
    my()->initial_time_local = millis();
    while(true){
        my()->current_time = millis();
        if (my()->current_time - my()->last_control_time >= my()->control_interval) {
            CanManager::enqueue_message(sender, type, message, msg_size);
            my()->last_control_time = millis();
        }
        CanManager::canBUS_to_actions_rotine(executeAction);
        // Check if the required number of acknowledgments has been received
        int ackCount = 0;
        for (unsigned char ack : my()->list_Ack_loopUntilACK) {
            ackCount++;
        }
        bool condition_typeEndGains = type != ENDGAINS || millis() - my()->initial_time_local > 500 ;
        if (ackCount >= nrOfAcknoledge && condition_typeEndGains) {
            break;
        }
    }
    


}

void CanManager::wake_up_grid() {
    // Initialize the number of nodes checked in with 1, since the current node is awake
    my()->nr_ckechIn_Nodes = 1;
    my()->list_Nr_detected_IDS.clear();
    
    // Record the initial time when the process started
    my()->initial_time_local = millis();

    my()->list_IDS.push_back((int)PICO_ID); //include my id in the list
    bool executeAction=true;
    unsigned char data[sizeof(int)];
    // Keep looping until all nodes are awake and conditions are met
    while (!CanManager::check_wake_up_condition()) {
        // Get the current time
        my()->current_time = millis();
        // If it's time to send a wake-up message
        if (my()->current_time - my()->last_control_time >= my()->control_interval) {
            // Prepare the data to be sent in the wake-up message
            
            memcpy(data, &my()->nr_ckechIn_Nodes, sizeof(int));
            // Enqueue the wake-up message to be sent to all nodes
            CanManager::enqueue_message(PICO_ID, my_type::WAKE_UP, data, sizeof(data));
            // Update the last control time
            my()->last_control_time = my()->current_time;
        }
        CanManager::canBUS_to_actions_rotine(executeAction);

    }   
    CanManager::enqueue_message(PICO_ID, my_type::WAKE_UP, data, sizeof(data));
    //Serial.println("Finished wake_up");
    Serial.print("Nr of found nodes ");Serial.println(my()->nr_ckechIn_Nodes);//Serial.print(" Nr id of my pico ");Serial.println((int)PICO_ID);

    std::vector<int> int_vector;

    // Convert char to int
    for (char c : my()->list_IDS) {
        int_vector.push_back(c - '0');
    }

    // Sort the vector
    std::sort(int_vector.begin(), int_vector.end());
    my()->list_IDS.clear();

    // Convert int to char
    for (int i : int_vector) {
        my()->list_IDS.push_back(i + '0');
    }

    // Initialize list_nodes with values ranging from 0 to the size of list_IDS
    for (int i = 0; i < my()->list_IDS.size(); i++) {
        my()->list_nodes.push_back(i);
    }

    for (size_t i = 0; i < my()->list_IDS.size(); ++i) {
        my()->id_to_node[my()->list_IDS[i]] = my()->list_nodes[i];
    }

    my()->THIS_NODE_NR = my()->id_to_node[(int)PICO_ID];

      // Create a map to associate node numbers with IDs
    for (auto const &pair : my()->id_to_node) {
        my()->node_to_id[pair.second] = pair.first;
    }
    my()->list_Ack.resize(my()->nr_ckechIn_Nodes - 1); //maximum nr of acknoledges is the size of the list
    my()->list_Ack_loopUntilACK.resize(my()->nr_ckechIn_Nodes - 1); //maximum nr of acknoledges is the size of the list

    
}


// Function to check if all nodes are awake and conditions are met
bool CanManager::check_wake_up_condition() {
    
    int auxiliar_de_Calculo = count(my()->list_Nr_detected_IDS.begin(), my()->list_Nr_detected_IDS.end(), my()->nr_ckechIn_Nodes);
    if ( auxiliar_de_Calculo== my()->list_Nr_detected_IDS.size() &&
        my()->nr_ckechIn_Nodes ==3 && millis() - my()->initial_time_local > 1000 ) { //
        return true;

    }
    else{ 
        return false;
    }
}


       

