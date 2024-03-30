#include "includes/CanManager.hpp"

void CanManager::createMap(void) {
    _actionMap[my_type::OFF] = offAction;
    _actionMap[my_type::ON] = onAction;
    _actionMap[my_type::ACK] = ackAction;
    _actionMap[my_type::SET_DUTY_CYCLE] = setDutyCycleAction;
    _actionMap[my_type::GET_DUTY_CYCLE] = getDutyCycleAction;
    _actionMap[my_type::SET_REFERENCE] = setReferenceAction;
    _actionMap[my_type::GET_REFERENCE] = getReferenceAction;
}

// 8*8 bits = 64bits
//          1       2       3      4      5      6       7       8     (bytes)
// DATA | sender | type |  INT    INT    INT    INT  |       |       |
void CanManager::enqueue_message(uint8_t sender, my_type type, uint8_t *message, std::size_t msg_size)
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
  rp2040.fifo.push_nb((uint32_t)new_frame);
}

void onAction(info_msg &msg) {
    analogWrite(my()->LED_PIN, 4095);
    enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

void offAction(info_msg &msg) {
    analogWrite(my()->LED_PIN, 0);
    enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

void setReferenceAction(info_msg &msg) {
    my()->x_ref = static_cast<float>(msg.data);
    enqueue_message(msg.sender, msg.type, nullptr, 0);
}

void getReferenceAction(info_msg &msg) {
    ;    
}