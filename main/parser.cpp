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
    if (command.length() == 0) {
        Serial.println("Command empty!");
        return;
    }
    switch (cmd) {
        case 'g': // Getters
            if (sscanf(command.c_str(), "g %c %f", &item, &val) == 2){
                Serial.println("ack");
                this->getters(item, val);
            }
            else{
                Serial.println("err");           
            }
            break;
        case 'r':
            if (sscanf(command.c_str(), "r %d %f", &i, &val) == 2){
                if (PICO_ID == HUB) {
                    my()->x_ref = val;
                    Serial.println("ack");
                }
                else {
                    uint8_t new_data[6] = {0};
                    memcpy(new_data, &(val), sizeof(val));
                    CanManager::enqueue_message(PICO_ID, my_type::SET_REFERENCE, new_data, sizeof(val));
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'd': 
            if (sscanf(command.c_str(), "d %d %f", &i, &val) == 2){
                my()->my_pid.setDutyCycle(val, 10); 
                Serial.println("ack");
                //CanManager::enqueue_message(i, my_type::SET_DUTY_CYCLE, val, sizeof(val));
            }
            else {
                Serial.println("err");
            }
            break;
        case 'o':
            if (sscanf(command.c_str(), "o %d %f", &i, &val) == 2){
                my()->occupancy = (bool)val;
                Serial.println("ack");
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
        case 'd':
            result = my()->u;
            break;
        case 'r':
            if (PICO_ID == HUB)
                Serial.printf("r %d %lf\n", PICO_ID, my()->x_ref);
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_REFERENCE, new_data, sizeof(val));
            }
            break;
        case 'o':
            result = my()->occupancy;
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
    Serial.print("Result: "); 
    if (flag == 1)
        Serial.println(value);
    else
        Serial.println(result);
    ;
}