#include "includes/distrControl.hpp"
#include "includes/CanManager.hpp"
#include "includes/vars.h"
#include "includes/my_aux.h"


distrControl *distrControl::instance = nullptr; // Initialize static instance pointer
float distrControl::gainsVector[4]; //nodes of picos are 0, 1, 2. In this vector, 
//the gains corresponding to node 0 are in entry 0, node 1 - entry 1, node 2 - entry 2. In entry 3 there is the external luminance
bool distrControl::endGAINS_bool = false;

void distrControl::setUpGains(){
    //gainsVector.resize(my()->nr_ckechIn_Nodes + 1);
    
    if(my()->THIS_NODE_NR == 0){
    
        //measure lux o - external - no light
        CanManager::loopUntilACK(my()->nr_ckechIn_Nodes-1 , CanManager::PICO_ID, my_type::MeasureNoLights, nullptr, 0);
        float vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        int index=my()->nr_ckechIn_Nodes;
        distrControl::gainsVector[index] = Volt2LUX(vss); //Get LDR value in lux
 
    
        analogWrite(my()->LED_PIN, 4000); //Apply control signal to LED
        unsigned char data[sizeof(int)];
        memcpy(data, &my()->THIS_NODE_NR, sizeof(int));
        CanManager::loopUntilACK(my()->nr_ckechIn_Nodes-1 , CanManager::PICO_ID, my_type::MeasureLights, data ,sizeof(data) );
        vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        float x_lux = Volt2LUX(vss); //Get LDR value in lux
        //            0 = my()->THIS_NODE_NR;                     index = my()->nr_ckechIn_Nodes
        distrControl::gainsVector[0] = (x_lux - distrControl::gainsVector[index]) / 4000;

        memcpy(data, &my()->THIS_NODE_NR + 1, sizeof(int));
        //this informs pico 1 that he should light up. From now on, pico 1 will be in charge
        CanManager::loopUntilACK(1 , CanManager::PICO_ID, my_type::NotifyFutureLight, data ,sizeof(data) );
        
    } 
    //loop non blocking until endGains_bool
    while(!endGAINS_bool){ 
        CanManager::canBUS_to_actions_rotine();

    }        
    
    Serial.println("End of setUP gains");

}






    