#include "includes/parser.hpp"
#include "includes/CanManager.hpp"
#include "includes/vars.h"
#include "includes/my_aux.h"
#include "includes/distrControl.hpp"

Parser::Parser() {}

Parser::~Parser() {}

void Parser::parseCommand(const String& command) {
    char cmd = command.charAt(0); // Get the command type
    int i; //Led number pin
    float val;
    char item;
    int nb;
    //Serial.println("----------NEW COMMAND---------");
    if (command.length() == 0) {
        Serial.println("Command empty!");
        return;
    }
    switch (cmd) {
        case 'g': // Getters
            if (command.startsWith("g b ")) { // Check if it's a buffer request
                char x;
                int i;
                if (sscanf(command.c_str(), "g b %c %d", &x, &i) == 2) {
                    // Call a function to handle buffer retrieval
                    
                    this->getBuffer(x, i);
                } 
                else {
                    Serial.println("err");
                }
            }
            else if (sscanf(command.c_str(), "g %c %f", &item, &val)){
                this->getters(item, val);
                //Serial.println("ack");
            }
            else {
                Serial.println("err");           
            }
            break;
        case 'r': //r 66 20 - (reference, pico_id, value)
            if (sscanf(command.c_str(), "r %d %f", &i, &val) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == HUB) {
                    my()->x_ref = val;
                    Serial.println("ack");
                    //Serial.print("New reference set: ");Serial.println(my()->x_ref); 
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
                    //Serial.print("New duty cycle set: "); Serial.println(val);
                    //Serial.print("Time: "); Serial.println(time);
                    Serial.println("ack");
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
                int id = my()->id_to_node[value];
                if (lil == 'l') {
                    //Serial.print("size: "); Serial.println(my()->list_stream_lux.size());
                    my()->list_stream_lux[id] = true;
                    Serial.println("ack");
                    //Serial.print("bool val: "); Serial.println(my()->list_stream_lux[id]);
                }
                else if (lil == 'd') {
                    //Serial.println("cheguei!");
                    my()->list_stream_duty_cycle[id] = true;
                    Serial.println("ack");
                    //Serial.print("bool val: "); Serial.println(my()->list_stream_duty_cycle[id]);
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
                int idS = my()->id_to_node[valueS]; 
                if (i_char == 'l') {
                    my()->list_stream_lux[idS] = false;
                    Serial.println("ack");
                    //Serial.print("S bool val: "); Serial.println(my()->list_stream_lux[idS]);
                }
                else if (i_char == 'd') {
                    my()->list_stream_duty_cycle[idS] = false;
                    Serial.println("ack");
                    //Serial.print("S bool val: "); Serial.println(my()->list_stream_duty_cycle[idS]);
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'O':
            if (sscanf(command.c_str(), "O %d %f", &i, &val) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == HUB) {
                    distrControl::set_lower_bound_occupied(val);
                    Serial.println("ack");
                }
                else {
                    uint8_t new_data[6] = {0};
                    memcpy(new_data, &(val), sizeof(val));
                    CanManager::enqueue_message(i_char, my_type::SET_LOWER_BOUND_OCCUPIED, new_data, sizeof(new_data));
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'U':
            if (sscanf(command.c_str(), "U %d %f", &i, &val) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == HUB) {
                    distrControl::set_lower_bound_unoccupied(val);
                    Serial.println("ack");
                }
                else {
                    uint8_t new_data[6] = {0};
                    memcpy(new_data, &(val), sizeof(val));
                    CanManager::enqueue_message(i_char, my_type::SET_LOWER_BOUND_UNOCCUPIED, new_data, sizeof(new_data));
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'c':
            if (sscanf(command.c_str(), "c %d %f", &i, &val) == 2){
                unsigned char i_char = static_cast<unsigned char>(i);
                if (i_char == HUB) {
                    distrControl::set_cost(val);
                    Serial.println("ack");
                }
                else {
                    uint8_t new_data[6] = {0};
                    memcpy(new_data, &(val), sizeof(val));
                    CanManager::enqueue_message(i_char, my_type::SET_CURRENT_ENERGY_COST, new_data, sizeof(new_data));
                }
            }
            else {
                Serial.println("err");
            }
            break;
        case 'R':
            for (int LL = 0; LL < my()->list_IDS.size(); LL++) {
                if (my()->list_IDS[LL] != HUB) {
                    Serial.print("Restarting node: "); Serial.println(my()->list_IDS[LL]);
                    CanManager::enqueue_message(my()->list_IDS[LL], my_type::RESTART, nullptr, 0);
                }
            }
            Serial.println("Restarting device...");
            watchdog_enable(1, 10); // Set the watchdog to trigger after 1ms
            while (true); // Loop forever until the watchdog resets the device
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
        case 'O': //LOWER BOUND OCCUPIED
            if (val == HUB) {
                Serial.printf("O %d %lf\n", PICO_ID, distrControl::get_lower_bound_occupied());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_LOWER_BOUND_OCCUPIED, new_data, sizeof(new_data));
            }
            break;
        case 'U': //LOWER BOUND UNOCCUPIED
            if (val == HUB) {
                Serial.printf("U %d %lf\n", PICO_ID, distrControl::get_lower_bound_unoccupied());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_LOWER_BOUND_UNOCCUPIED, new_data, sizeof(new_data));
            }
            break;
        case 'c': //CURRENT ENERGY COST
            if (val == HUB) {
                Serial.printf("C %d %lf\n", PICO_ID, distrControl::get_cost());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_CURRENT_ENERGY_COST, new_data, sizeof(new_data));
            }
            break;
        case 'L': ///CURRENT LOWER BOUND
            if (val == HUB) {
                Serial.printf("L %d %lf\n", PICO_ID, distrControl::get_lower_bound());
            }
            else {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(val), sizeof(val));
                CanManager::enqueue_message(PICO_ID, my_type::GET_CURRENT_LOWER_BOUND, new_data, sizeof(new_data));
            }
            break;
        default:
            Serial.println("err");
            break;
    }
}

void Parser::getBuffer(char x, int i) { //LAST_MINUTE_BUFFER
    //Serial.print("x: "); Serial.println(x);
    //Serial.print("i: "); Serial.println(i);
    int id = my()->id_to_node[i];
    if (x == 'l') {
        //Serial.println("entrei NO L ");
        my()->list_stream_last_minute_lux[my()->id_to_node[id]] = true;
        Serial.print("bool val: "); Serial.println(my()->list_stream_last_minute_lux[my()->id_to_node[id]]);
    }
    else if (x == 'd') {
        my()->list_stream_last_minute_duty_cycle[my()->id_to_node[id]] = true;
        Serial.print("bool val: "); Serial.println(my()->list_stream_last_minute_duty_cycle[my()->id_to_node[id]]);

    }
    else {
        Serial.println("err");
    }
}