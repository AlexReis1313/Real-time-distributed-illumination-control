#include "includes/CanManager.hpp"
#include "includes/my_aux.h"
#include "includes/distrControl.hpp"
#include "includes/vars.h"



void CanManager::createMap(void) {
    _actionMap[my_type::OFF] = offAction;
    _actionMap[my_type::ON] = onAction;
    _actionMap[my_type::ACK] = ackAction;
    
    _actionMap[my_type::ACKINTERNA] = ackInternalAction;
    
    
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
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID){
 
        if (find(my()->list_Ack.begin(), my()->list_Ack.end(), msg.sender) == my()->list_Ack.end()){// is there this id in the list?
            my()->list_Ack.push_back(msg.sender);
         

        }
    }


}


void CanManager::ackAction(info_msg &msg) {
    if (PICO_ID == HUB){
        Serial.println("ACTION::ACK Action received");
    }
    enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
}



void CanManager::measureNOlightAction(info_msg &msg) {
   my()->initial_time_local = millis();
   while(true){
        if (millis()-my()->initial_time_local >500  ){ //wait half a second for the LDR to stabilize
            if (my()->o_lux ==-1){//if variable was not yet defined
                float vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
                my()->o_lux = Volt2LUX(vss); //Get LDR value in lux
                }
            char type = 'i';
            CanManager::acknoledge(type, msg.sender);
            break;
        
   }
   }
   
} 

void CanManager::measurelightAction(info_msg &msg) {
   my()->initial_time_local = millis();
   while(true){
   
        if (millis()-my()->initial_time_local >500){ //wait half a second for the LDR to stabilize
                int data_as_int;
                memcpy(&data_as_int, msg.data, sizeof(int));
                if (distrControl::gainsVector[data_as_int] ==-1){
                
                    float vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
                    float x_lux = Volt2LUX(vss); //Get LDR value in lux
                    float gain = (x_lux - my()->o_lux) / 4000;
                    distrControl::gainsVector[data_as_int] = gain;
                }
                char type = 'i';
                CanManager::acknoledge(type, msg.sender); //intermal ack
                break;
            }
   
    }
}


void CanManager::NotifyThisLightAction(info_msg &msg) {

    int data_as_int;
    memcpy(&data_as_int, msg.data, sizeof(int));
    if (data_as_int == my()->THIS_NODE_NR ){
        Serial.println("I am now in charge of setup");

        char type = 'i';
        CanManager::acknoledge(type, msg.sender); //inform previous node that this node has taken over

        analogWrite(my()->LED_PIN, 4000); //Apply control signal to LED
        CanManager::loopUntilACK(my()->nr_ckechIn_Nodes-1 , CanManager::PICO_ID, my_type::MeasureLights, msg.data ,sizeof(msg.data ) );

        float vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        float x_lux = Volt2LUX(vss); //Get LDR value in lux

        float gain = (x_lux - my()->o_lux) / 4000;
        distrControl::gainsVector[data_as_int] = gain;

        analogWrite(my()->LED_PIN, 0);
        if(my()->THIS_NODE_NR==my()->nr_ckechIn_Nodes - 1){//I am the last node
            Serial.println("I was the last node - ending gains");
            CanManager::loopUntilACK( my()->nr_ckechIn_Nodes-1 , CanManager::PICO_ID, my_type::ENDGAINS, nullptr ,0 );
            distrControl::endGAINS_bool = true;
            

        }
        else{//pass it to the next pico
            unsigned char data[sizeof(int)];
            int next_node_nr = my()->THIS_NODE_NR + 1;
            memcpy(data, &next_node_nr, sizeof(int));
            Serial.print("Passing master token to next node. To node: "); Serial.println(next_node_nr);            
            //this informs pico 1 that he should light up. From now on, pico 1 will be in charge
            CanManager::loopUntilACK(1 , CanManager::PICO_ID, my_type::NotifyFutureLight, data ,sizeof(data) );
    
        }

    }

}
void CanManager::EndGainsAction(info_msg &msg) {
    distrControl::endGAINS_bool = true;
    char type = 'i';
    CanManager::acknoledge(type, msg.sender);
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


void CanManager::WakeUpAction(info_msg &msg) {
    // If the sender is not already in the list of IDs
    int data_as_int;
    memcpy(&data_as_int, msg.data, sizeof(int));
    if (find(my()->list_IDS.begin(), my()->list_IDS.end(), msg.sender) == my()->list_IDS.end()) {
        // Add the sender to the list of IDs
        Serial.println("I have found a new node");

        my()->list_IDS.push_back(msg.sender);
        
        // Add the number of detected nodes to the list
    
        my()->list_Nr_detected_IDS.push_back(data_as_int);
        
        // Update the number of checked in nodes
        my()->nr_ckechIn_Nodes = (int)my()->list_IDS.size();
    }
    else{
        int i = 0;
        for (unsigned char id : my()->list_IDS) {
            if (id == msg.sender) {
                // Store the index when the entry equals msg.sender
                my()->list_Nr_detected_IDS[i-1] = data_as_int; //i-1 because list_IDS has 3 entries (myself and 2 others) and my()->list_Nr_detected_IDS has only the 2 others
            }
            i++;
        }
    
    }
}
