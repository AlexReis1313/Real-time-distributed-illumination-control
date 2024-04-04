#include "includes/CanManager.hpp"
#include "includes/my_aux.h"
#include "includes/distrControl.hpp"
#include "includes/vars.h"

//STREAM AND BUFFER
void CanManager::serialStreamLuxAction(info_msg &msg) {
    //Serial.println("ACTION::SERIAL Stream Lux Action received");
    float value;

    //my()->pico_buffers[my()->id_to_node[msg.can_id]].addValueLux(my()->vss_lux);
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB && my()->list_stream_lux[my()->id_to_node[msg.can_id]]) {
        Serial.print("s l "); 
        Serial.print(msg.can_id); Serial.print(" "); 
        Serial.print(value); Serial.print(" ");
        Serial.println(millis() - my()->initial_time);
    }
    if (PICO_ID == HUB && my()->list_stream_last_minute_lux[my()->id_to_node[msg.can_id]]) {
        my()->pico_buffers[my()->id_to_node[msg.can_id]].printBufferLux();
        my()->list_stream_last_minute_lux[my()->id_to_node[msg.can_id]] = false;
    }
}


void CanManager::serialStreamDutyCycleAction(info_msg &msg) {
    //Serial.println("ACTION::SERIAL Stream Duty Cycle Action received");
    float value;

    //my()->pico_buffers[my()->id_to_node[msg.can_id]].addValueDutyCycle(my()->u);
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB && my()->list_stream_duty_cycle[my()->id_to_node[msg.can_id]]) {
        Serial.print("s d "); 
        Serial.print(msg.can_id); Serial.print(" "); 
        Serial.print(value); Serial.print(" ");
        Serial.println(millis() - my()->initial_time);
    }
    if (PICO_ID == HUB && my()->list_stream_last_minute_duty_cycle[my()->id_to_node[msg.can_id]]) {
        my()->pico_buffers[my()->id_to_node[msg.can_id]].printBufferDutyCycle();
        my()->list_stream_last_minute_duty_cycle[my()->id_to_node[msg.can_id]] = false;
    }
}

//STREAM
// void CanManager::startStreamLuxAction(info_msg &msg) {
//     //Serial.print("msg id: "); Serial.println(msg.can_id);
//     //Serial.print("PICO_ID: "); Serial.println(PICO_ID);
//     if (msg.can_id == PICO_ID) {
//         Serial.println("ACTION::Set Stream LUX Action received");
//         my()->stream_lux = true;
//         //CanManager::enqueue_message(PICO_ID, my_type::SERIAL_STREAM_LUX, msg.data, sizeof(msg.data);
//     }
// }

// void CanManager::startStreamDutyCycleAction(info_msg &msg) {
//     if (msg.can_id == PICO_ID) {
//         Serial.println("ACTION::Set Stream Duty Cycle Action received");
//         my()->stream_duty_cycle = true;
//         //CanManager::enqueue_message(PICO_ID, my_type::SERIAL_STREAM_DUTY_CYCLE, msg.data, sizeof(msg.data));
//     }
// }


// void CanManager::stopStreamDutyCycleAction(info_msg &msg) {
//     if (msg.can_id == PICO_ID) {
//         Serial.println("ACTION::Stop Stream Duty Cycle Action received");
//         my()->stream_duty_cycle = false;
//     }
// }

// void CanManager::stopStreamLuxAction(info_msg &msg) {
//     if (msg.can_id == PICO_ID) {
//         Serial.println("ACTION::Stop Stream Lux Action received");
//         my()->stream_lux = false;
//     }
// }

// void CanManager::serialStreamLuxAction(info_msg &msg) {
//     //Serial.println("ACTION::SERIAL Stream Lux Action received");
//     float value;
//     int id;
   
//     id = static_cast<int>(msg.can_id);
//     memcpy(&value, msg.data, sizeof(float));
//     if (PICO_ID == HUB) {
//         Serial.print("s l "); 
//         Serial.print(id); Serial.print(" "); 
//         Serial.print(value); Serial.print(" ");
//         Serial.println(millis() - my()->initial_time);
//     }
// }

// void CanManager::serialStreamDutyCycleAction(info_msg &msg) {
//     //Serial.println("ACTION::SERIAL Stream Duty Cycle Action received");
//     float value;
//     int id;

//     id = static_cast<int>(msg.can_id);
//     memcpy(&value, msg.data, sizeof(float));
//     if (PICO_ID == HUB) {
//         Serial.print("s l "); 
//         Serial.print(id); Serial.print(" "); 
//         Serial.print(value); Serial.print(" ");
//         Serial.println(millis() - my()->initial_time);
//     }
// }



//STREAM
// void CanManager::startStreamLuxAction(info_msg &msg) {
//     //Serial.print("msg id: "); Serial.println(msg.can_id);
//     //Serial.print("PICO_ID: "); Serial.println(PICO_ID);
//     if (msg.can_id == PICO_ID) {
//         Serial.println("ACTION::Set Stream LUX Action received");
//         my()->stream_lux = true;
//         //CanManager::enqueue_message(PICO_ID, my_type::SERIAL_STREAM_LUX, msg.data, sizeof(msg.data);
//     }
// }

// void CanManager::startStreamDutyCycleAction(info_msg &msg) {
//     if (msg.can_id == PICO_ID) {
//         Serial.println("ACTION::Set Stream Duty Cycle Action received");
//         my()->stream_duty_cycle = true;
//         //CanManager::enqueue_message(PICO_ID, my_type::SERIAL_STREAM_DUTY_CYCLE, msg.data, sizeof(msg.data));
//     }
// }

// void CanManager::stopStreamDutyCycleAction(info_msg &msg) {
//     if (msg.can_id == PICO_ID) {
//         Serial.println("ACTION::Stop Stream Duty Cycle Action received");
//         my()->stream_duty_cycle = false;
//     }
// }

// void CanManager::stopStreamLuxAction(info_msg &msg) {
//     if (msg.can_id == PICO_ID) {
//         Serial.println("ACTION::Stop Stream Lux Action received");
//         my()->stream_lux = false;
//     }
// }

// void CanManager::serialStreamLuxAction(info_msg &msg) {
//     //Serial.println("ACTION::SERIAL Stream Lux Action received");
//     float value;
//     int id;
   
//     id = static_cast<int>(msg.can_id);
//     id_to_node = get_id_to_node(id);
//     my()->pico_buffers[id_to_node]->addValueLux(value);
//     memcpy(&value, msg.data, sizeof(float));
//     if (PICO_ID == HUB && my()->list_stream_lux[id_to_node]) {
//         Serial.print("s l "); 
//         Serial.print(id); Serial.print(" "); 
//         Serial.print(value); Serial.print(" ");
//         Serial.println(millis() - my()->initial_time);
//     }
// }


// void CanManager::serialStreamDutyCycleAction(info_msg &msg) {
//     //Serial.println("ACTION::SERIAL Stream Duty Cycle Action received");
//     float value;
//     int id;

//     id = static_cast<int>(msg.can_id);
//     my()->pico_buffers[id_to_node]->addValueLux(value);
//     memcpy(&value, msg.data, sizeof(float));
//     if (PICO_ID == HUB && my()->list_stream_duty_cycle[id]) {
//         my()->list_stream_duty_cycle[id] = true;
//         Serial.print("s d "); 
//         Serial.print(id); Serial.print(" "); 
//         Serial.print(value); Serial.print(" ");
//         Serial.println(millis() - my()->initial_time);
//     }
// }
