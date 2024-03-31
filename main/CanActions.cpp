#include "includes/CanManager.hpp"
#include "includes/aux.h"

void CanManager::createMap(void) {
    _actionMap[my_type::OFF] = offAction;
    _actionMap[my_type::ON] = onAction;
    _actionMap[my_type::ACK] = ackAction;
    _actionMap[my_type::SET_DUTY_CYCLE] = setDutyCycleAction;
    _actionMap[my_type::GET_DUTY_CYCLE] = getDutyCycleAction;
    _actionMap[my_type::SET_REFERENCE] = setReferenceAction;
    _actionMap[my_type::GET_REFERENCE] = getReferenceAction;
}

void onAction(info_msg &msg) {
    analogWrite(my()->LED_PIN, 4095);
    CanManager::enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

void offAction(info_msg &msg) {
    analogWrite(my()->LED_PIN, 0);
    CanManager::enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

void setReferenceAction(info_msg &msg) { //msg.data is a uint8_t( unsigned char*)
    float x_ref_value;
    memcpy(&x_ref_value, msg.data, sizeof(float)); // Copy bytes into x_ref_value
    my()->x_ref = x_ref_value;
    my()->ref_volts = Volt2LUX(x_ref_value);

    unsigned char* messagePointer = msg.data;
    CanManager::enqueue_message(msg.sender, msg.type, &messagePointer, sizeof(float));
}

void getReferenceAction(info_msg &msg) {
    ;//CanManager::enqueue_message(&(msg.sender), msg.type, reinterpret_cast<unsigned char**>(my()->x_ref), sizeof(my()->x_ref));
}