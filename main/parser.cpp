#include "includes/parser.hpp"
#include "includes/CanManager.hpp"
#include "includes/vars.h"
#include "includes/my_aux.h"

Parser::Parser() {}

Parser::~Parser() {}

void Parser::parseCommand(const String& command) {
    char cmd = command.charAt(0); // Get the command type
    int i; //Led number pin
    float val;
    char item;
    Serial.println("----------NEW COMMAND---------");
    if (command.length() == 0) {
        Serial.println("Command empty!");
        return;
    }
    switch (cmd) {
        case 'g': // Getters
            if (sscanf(command.c_str(), "g %c %f", &item, &val) == 2){
                this->getters(item, val);
                //Serial.println("ack");
            }
            else{
                Serial.println("err");           
            }
            break;
        case 'r': //r 66 20 - (reference, pico_id, value)
            if (sscanf(command.c_str(), "r %d %f", &i, &val) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == HUB) {
                    my()->x_ref = val;
                    Serial.print("New reference set: ");Serial.println(my()->x_ref); 
                }
                else {
                    uint8_t new_data[6] = {0};
                    memcpy(new_data, &(val), sizeof(val));
                    CanManager::enqueue_message(i_char, my_type::SET_REFERENCE, new_data, sizeof(new_data));
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'd': 
            if (sscanf(command.c_str(), "d %d %f", &i, &val) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == HUB) {
                    float time = 1000;
                    my()->my_pid.setDutyCycle(val, 1000);
                    Serial.print("New duty cycle set: "); Serial.println(val);
                    Serial.print("Time: "); Serial.println(time);
                }
                else {
                    uint8_t new_data[6] = {0};
                    memcpy(new_data, &(val), sizeof(val));
                    CanManager::enqueue_message(i_char, my_type::SET_DUTY_CYCLE, new_data, sizeof(new_data));
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'o': // Set occupancy
            if (sscanf(command.c_str(), "o %d %f", &i, &val) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == HUB) {
                    my()->occupancy = (bool)val;
                    Serial.println("ack");
                }
                else {
                    uint8_t new_data[6] = {0};
                    memcpy(new_data, &(val), sizeof(val));
                    CanManager::enqueue_message(i_char, my_type::SET_OCCUPANCY, new_data, sizeof(new_data));
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'k': // Set feedback control
            if (sscanf(command.c_str(), "k %d %f", &i, &val) == 2){
                my()->my_pid.setFeedforward((bool)!val); 
                Serial.println("ack");
            }
            else {
                Serial.println("err");
            }
            break;
        case 'a':
            if (sscanf(command.c_str(), "a %d %f", &i, &val) == 2){
                my()->my_pid.setAntiWindup((bool)val);
                Serial.println("ack");
            }
            else {
                Serial.println("err");
            }
            break;
        default:
            Serial.println("err");
            break;
    }
}

void Parser::getters(char &item, int val) {
    int i = 0;
    bool value;
    float result;
    int flag = 0;
    switch (item) {
        case 'I':
            Serial.print("ID: "); Serial.println(PICO_ID);
        case 'd': //DUTY CYCLE
            if (val == HUB) {
                Serial.printf("d %d %lf\n", PICO_ID, my()->u);
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_DUTY_CYCLE, new_data, sizeof(new_data));
            }
            break;
        case 'r': //REFERENCE g r 66
            if (val == HUB) {
                Serial.printf("r %d %lf\n", PICO_ID, my()->x_ref);
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_REFERENCE, new_data, sizeof(new_data));
            }
            break;
        case 'l': //MEASURED LIGHT
            float light;
            if (val == HUB) {
                light = analogRead(my()->LED_PIN);
                Serial.printf("l %d %lf\n", PICO_ID, light);
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_ILUMINANCE, new_data, sizeof(new_data));
            }
            break;
        case 'o': //OCCUPANCY
            if (val == HUB) {
                Serial.printf("o %d %d\n", PICO_ID, my()->occupancy);
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_OCCUPANCY, new_data, sizeof(new_data));
            }
            break;
        case 'k':
            value = !my()->my_pid.getFeedforward();
            flag = 1;
            break;
        case 'a':
            value = my()->my_pid.getAntiWindup();
            flag = 1;
            break;
        case 'e':
            result = my()->my_metrics.getEnergyConsumption();
        case 'f':
            result = my()->my_metrics.getAverageFlicker();
        case 'v':
            result = my()->my_metrics.getVisibilityError();
        default:
            Serial.println("err");
            break;
    }
}