// Done by Duarte 30/03/2024
#include "includes/vars.h"
#include "includes/aux.h"
#include "includes/CanManager.hpp"

CanManager canBus = CanManager();

void actions_rotine() {
    can_frame *frame;

    while (rp2040.fifo.available()) {
        if (!rp2040.fifo.pop_nb((uint32_t *)&frame))
            break;
        info_msg pm = CanManager::extract_message(frame);
        my_type message_type = pm.type;
        unsigned char  sender = pm.sender;
        unsigned char* data = pm.data;
        
        std::map<int, eventFunction>::iterator it = canBus._actionMap.find(message_type);
        if (canBus._actionMap.find(message_type) != canBus._actionMap.end()){
            it->second(pm);
            Serial.print("Type: "); Serial.print(it->first);
        }
    }
    if (Serial.available()) 
    {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Remove any whitespace
        my()->my_parser.parseCommand(command);
    }
}

void controller_rotine() {
    my()->current_time = millis();
    if (my()->current_time - my()->last_control_time >= my()->control_interval) {
        
        my()->vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        my()->vss_lux = Volt2LUX(my()->vss); //Get LDR value in lux

        get_H_xref();
        get_H_x();

        my()->my_pid.setBcontroller((1 / (my()->H_xref * my()->gain * my()->k))); //Change b of controller based on the H_ref and H_x    
        my()->u = my()->my_pid.compute_control(my()->ref_volts, my()->vss); // Compute control
        analogWrite(my()->LED_PIN, (my()->u)); //Apply control signal to LED
        my()->my_pid.housekeep(my()->ref_volts, my()->vss);

        print_vars();
        //float percnt_dutycycle = ((my()->vss * 4095) / 3.3) / 4095;
        //my()->my_metrics.updateMetrics(my()->x_ref, my()->vss_lux , percnt_dutycycle);
        my()->last_control_time = my()->current_time;
    }
}

void setup() {
    //Setup serial communication
    Serial.begin(115200);
    analogReadResolution(12);
    analogWriteFreq(30000); //30KHz
    analogWriteRange(4096); //Max PWM
    canBus.flashIDsetup();

    //Setup controller, metrics and parser
    vars_setup();
}

void setup1() {
    canBus.flashIDsetup();
    canBus.begin(CAN_1000KBPS);
    canBus.setUpFiltersAndMasks();
}

void loop() {
    actions_rotine();
    controller_rotine();
}

void loop1() {
    canBus.sendMessage1to0();
}


