#include "includes/CanManager.hpp"
#include "includes/aux_functions.h"
#include "includes/vars.h"

void CanManager::createMap(void) {
    _actionMap[my_type::OFF] = offAction;
    _actionMap[my_type::ON] = onAction;
    _actionMap[my_type::ACK] = ackAction;
    //_actionMap[my_type::SET_DUTY_CYCLE] = setDutyCycleAction;
    //_actionMap[my_type::GET_DUTY_CYCLE] = getDutyCycleAction;
    _actionMap[my_type::SET_REFERENCE] = setReferenceAction;
    _actionMap[my_type::GET_REFERENCE] = getReferenceAction;
    _actionMap[my_type::SERIAL_GET_REFERENCE] = serialGetReferenceAction;
    _actionMap[my_type::FOUND_HUB] = foundHubAction;
}

void CanManager::ackAction(info_msg &msg) {
    if (PICO_ID == HUB)
        Serial.println("ack");
}
void CanManager::onAction(info_msg &msg) {
    analogWrite(my()->LED_PIN, 4095);
    CanManager::enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

void CanManager::offAction(info_msg &msg) {
    analogWrite(my()->LED_PIN, 0);
    CanManager::enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

void CanManager::setReferenceAction(info_msg &msg) { //msg.data is a uint8_t( unsigned char*)
    if (*reinterpret_cast<unsigned int*>(msg.data) == PICO_ID) {
        float x_ref_value;
        memcpy(&x_ref_value, msg.data, sizeof(float)); // Copy bytes into x_ref_value
        my()->x_ref = x_ref_value;
        my()->ref_volts = Volt2LUX(x_ref_value);
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
    }
}

void CanManager::getReferenceAction(info_msg &msg) {
    if (*reinterpret_cast<unsigned int*>(msg.data) == PICO_ID) {
        msg.type = my_type::SERIAL_GET_REFERENCE;
        float x_ref_value = my()->x_ref;
        unsigned char data[sizeof(float)];
        memcpy(data, &x_ref_value, sizeof(float));
        CanManager::enqueue_message(PICO_ID, msg.type, data, sizeof(float));
    }
}

void CanManager::serialGetReferenceAction(info_msg &msg) {
    if (PICO_ID == HUB)
        Serial.printf("r %d %lf\n", msg.sender, msg.data, sizeof(float));
}

void CanManager::foundHubAction(info_msg &msg) {
    //bool hubValue =true;
    //memcpy(&hubValue, msg.data, sizeof(bool));
    HUB = msg.sender;
    CanManager::hubFound = true;
}