#include "includes/CanManager.hpp"
#include "includes/my_aux.h"
#include "includes/distrControl.hpp"
#include "includes/vars.h"

void CanManager::createMap(void) {
    //ACK
    _actionMap[my_type::ACK] = ackAction;
    _actionMap[my_type::ACKINTERNA] = ackInternalAction;

    //SETTERS
    _actionMap[my_type::SET_REFERENCE] = setReferenceAction;
    _actionMap[my_type::SET_DUTY_CYCLE] = setDutyCycleAction;
    _actionMap[my_type::SET_OCCUPANCY] = setOccupancyAction; 
    _actionMap[my_type::SET_ANTI_WINDDUP] = setAntiWindupAction;
    _actionMap[my_type::SET_FEEDBACK] = setFeedbackAction;

    //STREAM
    _actionMap[my_type::START_STREAM_LUX] = startStreamLuxAction;
    _actionMap[my_type::START_STREAM_DUTY_CYCLE] = startStreamDutyCycleAction;
    _actionMap[my_type::STOP_STREAM_LUX] = stopStreamLuxAction;
    _actionMap[my_type::STOP_STREAM_DUTY_CYCLE] = stopStreamDutyCycleAction;

    //GETTERS
    _actionMap[my_type::GET_REFERENCE] = getReferenceAction;
    _actionMap[my_type::GET_DUTY_CYCLE] = getDutyCycleAction;
    _actionMap[my_type::GET_ILUMINANCE] = getIluminanceAction;
    _actionMap[my_type::GET_OCCUPANCY] = getOccupancyAction;
    _actionMap[my_type::GET_ANTI_WINDDUP] = getAntiWindupAction;
    _actionMap[my_type::GET_FEEDBACK] = getFeedbackAction;
    _actionMap[my_type::GET_EXTERNAL_ILUMINANCE] = getExternalIluminanceAction;
    _actionMap[my_type::GET_ELAPSED_TIME] = getElapsedTimeAction;
    _actionMap[my_type::GET_INSTANTANEOUS_POWER] = getInstantaneousPowerAction;
    _actionMap[my_type::GET_AVERAGE_ENERGY] = getAverageEnergyAction;
    _actionMap[my_type::GET_AVERAGE_VISIBILITY] = getAverageVisibilityAction;
    _actionMap[my_type::GET_AVERAGE_FLICKER] = getAverageFlickerAction;

    //SERIALS
    _actionMap[my_type::SERIAL_GET_REFERENCE] = serialGetReferenceAction;
    _actionMap[my_type::SERIAL_GET_DUTY_CYCLE] = serialGetDutyCycleAction;
    _actionMap[my_type::SERIAL_GET_ILUMINANCE] = serialGetIluminanceAction;
    _actionMap[my_type::SERIAL_GET_OCCUPANCY] = serialGetOccupancyAction; 
    _actionMap[my_type::SERIAL_GET_ANTI_WINDDUP] = serialGetAntiWindupAction;
    _actionMap[my_type::SERIAL_GET_FEEDBACK] = serialGetFeedbackAction;
    _actionMap[my_type::SERIAL_GET_EXTERNAL_ILUMINANCE] = serialGetExternalIluminanceAction;
    _actionMap[my_type::SERIAL_GET_ELAPSED_TIME] = serialGetElapsedTimeAction;
    _actionMap[my_type::SERIAL_GET_INSTANTANEOUS_POWER] = serialGetAverageFlickerAction;
    _actionMap[my_type::SERIAL_GET_AVERAGE_ENERGY] = serialGetAverageEnergyAction;
    _actionMap[my_type::SERIAL_GET_AVERAGE_VISIBILITY] = serialGetAverageVisibilityAction;
    _actionMap[my_type::SERIAL_GET_AVERAGE_FLICKER] = serialGetAverageFlickerAction;
    _actionMap[my_type::SERIAL_STREAM_LUX] = serialStreamLuxAction;
    _actionMap[my_type::SERIAL_STREAM_DUTY_CYCLE] = serialStreamDutyCycleAction;

    //HUB
    _actionMap[my_type::FOUND_HUB] = foundHubAction;

    //WAKE UP
    _actionMap[my_type::WAKE_UP] = WakeUpAction;
    _actionMap[my_type::MEASURE_NO_LIGHTS] = measureNOlightAction;
    _actionMap[my_type::MEASURE_LIGHTS] = measurelightAction;
    _actionMap[my_type::NOTIFY_FUTURE_LIGHT] = NotifyThisLightAction;
    _actionMap[my_type::ENDGAINS] = EndGainsAction;
}

void CanManager::ackInternalAction(info_msg &msg) {
    int node = my()->id_to_node[msg.sender];
    my()->list_Ack[node] = true;
}

void CanManager::ackAction(info_msg &msg) {
    if (PICO_ID == HUB){
        Serial.println("ACTION::ACK Action received");
    }
    //enqueue_message(msg.sender, my_type::ACK, nullptr, 0);
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

void CanManager::setDutyCycleAction(info_msg &msg) {
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Duty Cycle Action received");
        float time = 1; //time in seconds
        my()->my_pid.setDutyCycle(value, time);
        Serial.print("New duty cycle set: "); Serial.println(value);
        Serial.print("Time (s): "); Serial.println(time);
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
    }
}

void CanManager::setOccupancyAction(info_msg &msg) {
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Occupancy Action received");
        my()->occupancy = (bool)value;
        Serial.print("New occupancy set: "); Serial.println((bool)value);
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
        
    }
}

void CanManager::setAntiWindupAction(info_msg &msg) {
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Anti Windup Action received");
        my()->my_pid.setAntiWindup((bool)value);
        Serial.print("New anti windup set: "); Serial.println(value);
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
    }
}         

void CanManager::setFeedbackAction(info_msg &msg) {
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Feedback Action received");
        my()->my_pid.setFeedback((bool)value);
        Serial.print("New feedback set: "); Serial.println(value);
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
    }
}

//STREAM
void CanManager::startStreamLuxAction(info_msg &msg) {
    //Serial.print("msg id: "); Serial.println(msg.can_id);
    //Serial.print("PICO_ID: "); Serial.println(PICO_ID);
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Stream LUX Action received");
        my()->stream_lux = true;
        //CanManager::enqueue_message(PICO_ID, my_type::SERIAL_STREAM_LUX, msg.data, sizeof(msg.data);
    }
}

void CanManager::startStreamDutyCycleAction(info_msg &msg) {
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Stream Duty Cycle Action received");
        my()->stream_duty_cycle = true;
        //CanManager::enqueue_message(PICO_ID, my_type::SERIAL_STREAM_DUTY_CYCLE, msg.data, sizeof(msg.data));
    }
}

void CanManager::serialStreamLuxAction(info_msg &msg) {
    //Serial.println("ACTION::SERIAL Stream Lux Action received");
    float value;
    int id;
   
    id = static_cast<int>(msg.can_id);
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.print("s l "); 
        Serial.print(id); Serial.print(" "); 
        Serial.print(value); Serial.print(" ");
        Serial.println(millis() - my()->initial_time);
    }
}

void CanManager::stopStreamDutyCycleAction(info_msg &msg) {
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Stop Stream Duty Cycle Action received");
        my()->stream_duty_cycle = false;
    }
}

void CanManager::stopStreamLuxAction(info_msg &msg) {
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Stop Stream Lux Action received");
        my()->stream_lux = false;
    }
}

void CanManager::serialStreamDutyCycleAction(info_msg &msg) {
    //Serial.println("ACTION::SERIAL Stream Duty Cycle Action received");
    float value;
    int id;

    id = static_cast<int>(msg.can_id);
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.print("s l "); 
        Serial.print(id); Serial.print(" "); 
        Serial.print(value); Serial.print(" ");
        Serial.println(millis() - my()->initial_time);
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
        CanManager::enqueue_message(PICO_ID, msg.type, data, sizeof(data));
    }
}

void CanManager::getDutyCycleAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Duty Cycle Action received");
        msg.type = my_type::SERIAL_GET_DUTY_CYCLE;
        unsigned char data[sizeof(float)];
        memcpy(data, &my()->u, sizeof(my()->u));
        CanManager::enqueue_message(PICO_ID, msg.type, data, sizeof(data));
    }
}

void CanManager::getIluminanceAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Iluminance Action received");
        unsigned char data[sizeof(float)];
        memcpy(data, &my()->vss_lux, sizeof(my()->vss_lux));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_ILUMINANCE, data, sizeof(data));
    }
}

void CanManager::getOccupancyAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Occupancy Action received");
        unsigned char data[sizeof(float)];
        memcpy(data, &my()->occupancy, sizeof(my()->occupancy));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_OCCUPANCY, data, sizeof(data));
    }
}

void CanManager::getAntiWindupAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Anti Windup Action received");
        unsigned char data[sizeof(float)];
        bool read_val = my()->my_pid.getAntiWindup();
        memcpy(data, &read_val, sizeof(read_val));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_ANTI_WINDDUP, data, sizeof(data));
    }
}

void CanManager::getFeedbackAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Feedback Action received");
        unsigned char data[sizeof(float)];
        bool read_val = my()->my_pid.getFeedback();
        memcpy(data, &read_val, sizeof(read_val));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_FEEDBACK, data, sizeof(data));
    }
}

void CanManager::getExternalIluminanceAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get External Iluminance Action received");
        unsigned char data[sizeof(float)];
        float light = my()->o_lux;
        memcpy(data, &light, sizeof(light));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_EXTERNAL_ILUMINANCE, data, sizeof(data));
    }
}

void CanManager::getElapsedTimeAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Elapsed Time Action received");
        unsigned char data[sizeof(float)];
        float delta = (float)(my()->current_time - my()->initial_time) * std::pow(10, -3);
        memcpy(data, &delta, sizeof(delta));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_ELAPSED_TIME, data, sizeof(data));
    }
}

void CanManager::getInstantaneousPowerAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Instantaneous Power Action received");
        unsigned char data[sizeof(float)];
        float power = my()->my_metrics.getInstantPower();
        memcpy(data, &power, sizeof(power));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_INSTANTANEOUS_POWER, data, sizeof(data));
    }
}

void CanManager::getAverageEnergyAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Average Energy Action received");
        unsigned char data[sizeof(float)];
        float energy = my()->my_metrics.getEnergyConsumption();
        memcpy(data, &energy, sizeof(energy));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_AVERAGE_ENERGY, data, sizeof(data));
    }
}

void CanManager::getAverageVisibilityAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Average Visibility Action received");
        unsigned char data[sizeof(float)];
        float visibility = my()->my_metrics.getVisibilityError();
        memcpy(data, &visibility, sizeof(visibility));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_AVERAGE_VISIBILITY, data, sizeof(data));
    }
}

void CanManager::getAverageFlickerAction(info_msg &msg) {
    int value;
    memcpy(&value, msg.data, sizeof(int));
    if (value == PICO_ID) {
        Serial.println("ACTION::Get Average Flicker Action received");
        unsigned char data[sizeof(float)];
        float flicker = my()->my_metrics.getAverageFlicker();
        memcpy(data, &flicker, sizeof(flicker));
        CanManager::enqueue_message(PICO_ID, my_type::SERIAL_GET_AVERAGE_FLICKER, data, sizeof(data));
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

void CanManager::serialGetDutyCycleAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Duty Cycle Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("d %d %lf\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetIluminanceAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Iluminance Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("l %d %lf\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetOccupancyAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Occupancy Action received");
    bool value;
    memcpy(&value, msg.data, sizeof(bool));
    if (PICO_ID == HUB) {
        Serial.printf("o %d %d\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetAntiWindupAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Anti Windup Action received");
    bool value;
    memcpy(&value, msg.data, sizeof(bool));
    if (PICO_ID == HUB) {
        Serial.printf("a %d %d\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetFeedbackAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Feedback Action received");
    bool value;
    memcpy(&value, msg.data, sizeof(bool));
    if (PICO_ID == HUB) {
        Serial.printf("f %d %d\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetExternalIluminanceAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get External Iluminance Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("e %d %lf\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetElapsedTimeAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Elapsed Time Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("t %d %lf\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetInstantaneousPowerAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Instantaneous Power Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("p %d %lf\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetAverageEnergyAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Average Energy Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("e %d %lf\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetAverageVisibilityAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Average Visibility Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("v %d %lf\n", msg.sender, value);
        Serial.println("ack");
    }
}

void CanManager::serialGetAverageFlickerAction(info_msg &msg) {
    Serial.println("ACTION::SERIAL Get Average Flicker Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("f %d %lf\n", msg.sender, value);
        Serial.println("ack");
    }
}

// HUB
void CanManager::foundHubAction(info_msg &msg) {
    Serial.println("ACTION::Found Hub Action received");
    HUB = msg.sender;
    CanManager::hubFound = true;
}


//WAKE UP
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

void CanManager::NotifyThisLightAction(info_msg &msg) {
    int data_as_int;
    memcpy(&data_as_int, msg.data, sizeof(int));
    if (data_as_int == my()->THIS_NODE_NR ){
        char type = 'i';
        CanManager::acknoledge(type); //inform previous node that this node has taken over

        analogWrite(my()->LED_PIN, 4000); //Apply control signal to LED
        CanManager::loopUntilACK(my()->nr_ckechIn_Nodes-1 , CanManager::PICO_ID, my_type::MEASURE_LIGHTS, msg.data ,sizeof(msg.data ) );

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
            CanManager::loopUntilACK(1 , CanManager::PICO_ID, my_type::NOTIFY_FUTURE_LIGHT, data ,sizeof(data) );
        }
    }
}
void CanManager::EndGainsAction(info_msg &msg) {
    distrControl::endGAINS_bool = true;
    char type = 'i';
    CanManager::acknoledge(type);

}