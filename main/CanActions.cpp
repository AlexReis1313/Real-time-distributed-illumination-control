#include "includes/CanManager.hpp"
#include "includes/my_aux.h"
#include "includes/vars.h"

void CanManager::createMap(void) {
    _actionMap[my_type::OFF] = offAction;
    _actionMap[my_type::ON] = onAction;
    _actionMap[my_type::ACK] = ackAction;
    _actionMap[my_type::SET_DUTY_CYCLE] = setDutyCycleAction;
    _actionMap[my_type::GET_DUTY_CYCLE] = getDutyCycleAction;
    _actionMap[my_type::SET_REFERENCE] = setReferenceAction;
    _actionMap[my_type::GET_REFERENCE] = getReferenceAction;
    _actionMap[my_type::SERIAL_GET_REFERENCE] = serialGetReferenceAction;
    _actionMap[my_type::FOUND_HUB] = foundHubAction;
}

void CanManager::ackAction(info_msg &msg) {
    if (PICO_ID == HUB){
        Serial.println("ACTION::ACK Action received");
    }
    enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}
void CanManager::onAction(info_msg &msg) {
    Serial.println("ACTION::ON Action received");
    analogWrite(my()->LED_PIN, 4095);
    CanManager::enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

void CanManager::offAction(info_msg &msg) {
    Serial.println("ACTION::OFF Action received");
    analogWrite(my()->LED_PIN, 0);
    CanManager::enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

//Setters

void CanManager::setReferenceAction(info_msg &msg) { //msg.data is a uint8_t( unsigned char*)
    float value;
    memcpy(&value, msg.data, sizeof(float));
    //Serial.print("id: "); Serial.println(msg.can_id);
    //Serial.print("value: "); Serial.println(value);
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Reference Action received---");
        float x_ref_value;
        memcpy(&x_ref_value, msg.data, sizeof(float)); // Copy bytes into x_ref_value
        my()->x_ref = x_ref_value;
        my()->ref_volts = Volt2LUX(x_ref_value);
        Serial.print("New reference set: "); Serial.println(my()->x_ref); 
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
    }
}

//Getters
void CanManager::getReferenceAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    //Serial.print("msg.data: "); Serial.println(value);
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Reference Action received----");
        msg.type = my_type::SERIAL_GET_REFERENCE;
        //Serial.print("x_ref_value: "); Serial.println(my()->x_ref);
        unsigned char data[sizeof(float)];
        memcpy(data, &my()->x_ref, sizeof(my()->x_ref));
        //Serial.println("------------------------------");
        CanManager::enqueue_message(PICO_ID, msg.type, data, sizeof(data));
    }
}

void CanManager::setDutyCycleAction(info_msg &msg) {
    Serial.println("ACTION::Set Duty Cycle Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    my()->my_pid.setDutyCycle(value, 1000);
    CanManager::enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}


// Serials

void CanManager::serialGetReferenceAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Reference Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("r %d %lf\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::foundHubAction(info_msg &msg) {
    Serial.println("ACTION::Found Hub Action received");
    HUB = msg.sender;
    CanManager::hubFound = true;
}
