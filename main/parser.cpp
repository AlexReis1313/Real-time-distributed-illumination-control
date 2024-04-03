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
    //Serial.println("----------NEW COMMAND---------");
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
                    float time = 1;
                    my()->my_pid.setDutyCycle(val, 1);
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
        case 'a':
            if (sscanf(command.c_str(), "a %d %f", &i, &val) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == HUB) {
                    my()->my_pid.setAntiWindup((bool)val);
                    Serial.println("ack");
                }
                else {
                    uint8_t new_data[6] = {0};
                    memcpy(new_data, &(val), sizeof(val));
                    CanManager::enqueue_message(i_char, my_type::SET_ANTI_WINDDUP, new_data, sizeof(new_data));
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'k': // Set feedback control
            if (sscanf(command.c_str(), "k %c %d", &i, &val) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == HUB) {
                    my()->my_pid.setFeedback((bool)val);
                    Serial.println("ack");
                }
                else {
                    uint8_t new_data[6] = {0};
                    memcpy(new_data, &(val), sizeof(val));
                    CanManager::enqueue_message(i_char, my_type::SET_FEEDBACK, new_data, sizeof(new_data));
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 's': //Start stream
            int value;
            if (sscanf(command.c_str(), "s %c %d", &i, &value) == 2){
                unsigned char lil = static_cast<unsigned char>(i);
                if (lil == 'l') {
                    if (value == HUB) {
                        my()->stream_lux = true;
                    }
                    else {
                        CanManager::enqueue_message(value, my_type::START_STREAM_LUX, nullptr, 0);
                    }
                }
                else if (lil == 'd') {
                    if (value == HUB) {
                        my()->stream_duty_cycle = true;
                    }
                    else {
                        CanManager::enqueue_message(value, my_type::START_STREAM_DUTY_CYCLE, nullptr, 0);
                    }
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'S': //Stop stream
            int valueS;
            if (sscanf(command.c_str(), "S %c %d", &i, &valueS) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == 'l') {
                    if (valueS == HUB) {
                        my()->stream_lux = false;
                    }
                    else {
                        CanManager::enqueue_message(valueS, my_type::STOP_STREAM_LUX, nullptr, 0);
                    }
                }
                else if (i_char == 'd') {
                    if (valueS == HUB) {
                        my()->stream_duty_cycle = false;
                    }
                    else {
                        CanManager::enqueue_message(valueS, my_type::STOP_STREAM_DUTY_CYCLE, nullptr, 0);
                    }
                }
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
    switch (item) {
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
                Serial.printf("l %d %lf\n", PICO_ID, my()->vss_lux);
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
        case 'a': //ANTI-WINDUP
            if (val == HUB) {
                Serial.printf("a %d %d\n", PICO_ID, my()->my_pid.getAntiWindup());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_ANTI_WINDDUP, new_data, sizeof(new_data));
            }
            break;
        case 'k': //FEEDBACK
            if (val == HUB) {
                Serial.printf("k %d %d\n", PICO_ID, my()->my_pid.getFeedback());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_FEEDBACK, new_data, sizeof(new_data));
            }
            break;
        case 'x': //EXTERNAL LIGHT
            if (val == HUB) {
                Serial.printf("x %d %lf\n", PICO_ID, my()->o_lux);
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_EXTERNAL_ILUMINANCE, new_data, sizeof(new_data));
            }
            break;
        case 't': //ELAPSED TIME
            if (val == HUB) {
                float delta = (float)(my()->current_time - my()->initial_time) * std::pow(10, -3);
                Serial.printf("t %d %lf\n", PICO_ID,  delta);
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_ELAPSED_TIME, new_data, sizeof(new_data));
            }
            break;
        case 'p': //INSTANTANEOUS POWER
            if (val == HUB) {
                Serial.printf("p %d %lf\n", PICO_ID, my()->my_metrics.getInstantPower());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_INSTANTANEOUS_POWER, new_data, sizeof(new_data));
            }  
            break;  
        case 'e': //AVERAGE ENERGY
            if (val == HUB) {
                Serial.printf("e %d %lf\n", PICO_ID, my()->my_metrics.getEnergyConsumption());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_AVERAGE_ENERGY, new_data, sizeof(new_data));
            }
            break;
        case 'v': //AVERAGE VISIBILITY
            if (val == HUB) {
                Serial.printf("v %d %lf\n", PICO_ID, my()->my_metrics.getVisibilityError());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_AVERAGE_VISIBILITY, new_data, sizeof(new_data));
            }
            break;
        case 'f': //AVERAGE FLICKER
            if (val == HUB) {
                Serial.printf("f %d %lf\n", PICO_ID, my()->my_metrics.getAverageFlicker());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_AVERAGE_FLICKER, new_data, sizeof(new_data));
            }
            break;
        default:
            Serial.println("err");
            break;
    }
}