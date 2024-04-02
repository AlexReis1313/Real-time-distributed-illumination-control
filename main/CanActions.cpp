#include "includes/CanManager.hpp"
#include "includes/my_aux.h"
#include "includes/distrControl.hpp"
#include "includes/vars.h"



void CanManager::createMap(void) {
    _actionMap[my_type::OFF] = offAction;
    _actionMap[my_type::ON] = onAction;
    _actionMap[my_type::ACK] = ackAction;
    _actionMap[my_type::ACKINTERNA] = ackInternalAction;
    //_actionMap[my_type::SET_DUTY_CYCLE] = setDutyCycleAction;
    //_actionMap[my_type::GET_DUTY_CYCLE] = getDutyCycleAction;
    _actionMap[my_type::SET_REFERENCE] = setReferenceAction;
    _actionMap[my_type::GET_REFERENCE] = getReferenceAction;
    _actionMap[my_type::SERIAL_GET_REFERENCE] = serialGetReferenceAction;
    _actionMap[my_type::FOUND_HUB] = foundHubAction;
    _actionMap[my_type::WAKE_UP] = WakeUpAction;
   _actionMap[my_type::MeasureNoLights] = measureNOlightAction;
   _actionMap[my_type::MeasureLights] = measurelightAction;
   _actionMap[my_type::NotifyFutureLight] = NotifyThisLightAction;
   _actionMap[my_type::ENDGAINS] = EndGainsAction;

}

void CanManager::ackInternalAction(info_msg &msg) {
    int node = my()->id_to_node[msg.sender];
    my()->list_Ack[node] = true;
}

void CanManager::ackAction(info_msg &msg) {
    if (PICO_ID == HUB)
        Serial.println("ack");
}


void CanManager::measureNOlightAction(info_msg &msg) {
   my()->initial_time = millis();
   if (millis()-my()->initial_time >500){ //wait half a second for the LDR to stabilize
        float vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        int index = my()->nr_ckechIn_Nodes;
        distrControl::gainsVector[index] = Volt2LUX(vss); //Get LDR value in lux
        char type = 'i';
        CanManager::acknoledge(type);
   }
   
} 

void CanManager::NotifyThisLightAction(info_msg &msg) {
    int data_as_int;
    memcpy(&data_as_int, msg.data, sizeof(int));
    if (data_as_int == my()->THIS_NODE_NR ){
        char type = 'i';
        CanManager::acknoledge(type); //inform previous node that this node has taken over

        analogWrite(my()->LED_PIN, 4000); //Apply control signal to LED
        CanManager::loopUntilACK(my()->nr_ckechIn_Nodes-1 , CanManager::PICO_ID, my_type::MeasureLights, msg.data ,sizeof(msg.data ) );

        float vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        float x_lux = Volt2LUX(vss); //Get LDR value in lux

        distrControl::gainsVector[data_as_int] = (x_lux - distrControl::gainsVector[my()->nr_ckechIn_Nodes]) / 4000;
        
        if(my()->THIS_NODE_NR==my()->nr_ckechIn_Nodes - 1){//I am the last node
            CanManager::loopUntilACK( my()->nr_ckechIn_Nodes-1 , CanManager::PICO_ID, my_type::ENDGAINS, nullptr ,0 );
            distrControl::endGAINS_bool = true;
            

        }
        else{//pass it to the next pico
            unsigned char data[sizeof(int)];
            memcpy(data, &my()->THIS_NODE_NR + 1, sizeof(int));
            //this informs pico 1 that he should light up. From now on, pico 1 will be in charge
            CanManager::loopUntilACK(1 , CanManager::PICO_ID, my_type::NotifyFutureLight, data ,sizeof(data) );
    
        }

    }

}
void CanManager::EndGainsAction(info_msg &msg) {
    distrControl::endGAINS_bool = true;
    char type = 'i';
    CanManager::acknoledge(type);

}


void CanManager::measurelightAction(info_msg &msg) {
   my()->initial_time = millis();
   if (millis()-my()->initial_time >500){ //wait half a second for the LDR to stabilize
        float vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        float x_lux = Volt2LUX(vss); //Get LDR value in lux
        int data_as_int;
        memcpy(&data_as_int, msg.data, sizeof(int));
        int index=my()->nr_ckechIn_Nodes;
        distrControl::gainsVector[data_as_int] = (x_lux - distrControl::gainsVector[index]) / 4000;
        char type = 'i';
        CanManager::acknoledge(type); //intermal ack
   }
   
}
void CanManager::onAction(info_msg &msg) {
    Serial.println("ON Action received");
    analogWrite(my()->LED_PIN, 4095);
    CanManager::enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

void CanManager::offAction(info_msg &msg) {
    Serial.println("OFF Action received");
    analogWrite(my()->LED_PIN, 0);
    CanManager::enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}

void CanManager::setReferenceAction(info_msg &msg) { //msg.data is a uint8_t( unsigned char*)
    Serial.println("Set Reference Action received");
    if (*reinterpret_cast<unsigned int*>(msg.data) == PICO_ID) {
        float x_ref_value;
        memcpy(&x_ref_value, msg.data, sizeof(float)); // Copy bytes into x_ref_value
        my()->x_ref = x_ref_value;
        my()->ref_volts = Volt2LUX(x_ref_value);
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
    }
}

void CanManager::getReferenceAction(info_msg &msg) {
    Serial.println("Get Reference Action received");
    if (*reinterpret_cast<unsigned int*>(msg.data) == PICO_ID) {
        msg.type = my_type::SERIAL_GET_REFERENCE;
        float x_ref_value = my()->x_ref;
        unsigned char data[sizeof(float)];
        memcpy(data, &x_ref_value, sizeof(float));
        CanManager::enqueue_message(PICO_ID, msg.type, data, sizeof(float));
    }
}

void CanManager::serialGetReferenceAction(info_msg &msg) {
    Serial.println("Serial Get Reference Action received");
    if (PICO_ID == HUB)
        Serial.printf("r %d %lf\n", msg.sender, msg.data, sizeof(float));
}

void CanManager::foundHubAction(info_msg &msg) {
    Serial.println("Found Hub Action received");
    //bool hubValue =true;
    //memcpy(&hubValue, msg.data, sizeof(bool));
    HUB = msg.sender;
    CanManager::hubFound = true;
}


void CanManager::WakeUpAction(info_msg &msg) {
    // If the sender is not already in the list of IDs
    if (find(my()->list_IDS.begin(), my()->list_IDS.end(), msg.sender) == my()->list_IDS.end()) {
        // Add the sender to the list of IDs
        my()->list_IDS.push_back(msg.sender);
        
        // Add the number of detected nodes to the list

        int data_as_int;
        memcpy(&data_as_int, msg.data, sizeof(int));
    
        my()->list_Nr_detected_IDS.push_back(data_as_int);
        
        // Update the number of checked in nodes
        my()->nr_ckechIn_Nodes = (int)my()->list_IDS.size();
    }
}