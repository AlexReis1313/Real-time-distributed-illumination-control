// Done by Duarte 30/03/2024
#include "includes/vars.h"
#include "includes/my_aux.h"
#include "includes/CanManager.hpp"
#include "includes/distrControl.hpp"


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

        //print_vars();
        float percnt_dutycycle = (my()->u) / 4095;
        my()->my_metrics.updateMetrics(my()->x_ref, my()->vss_lux , percnt_dutycycle);
        my()->last_control_time = my()->current_time;
    }
}

void stream_rotine() {
    if (my()->stream_lux) {
        if (PICO_ID == HUB) {
            Serial.print("s l ");
            Serial.print(PICO_ID); Serial.print(" ");
            Serial.print(my()->vss_lux); Serial.print(" ");
            Serial.println(millis() - my()->initial_time);
        }
        else {
            if (my()->current_time - my()->last_time_stream >= 50) {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(my()->vss_lux), sizeof(my()->vss_lux));
                CanManager::enqueue_message(PICO_ID, my_type::SERIAL_STREAM_LUX, new_data, sizeof(new_data));
                my()->last_time_stream = my()->current_time;
            }
        }
    }
    if (my()->stream_duty_cycle) {
        if (PICO_ID == HUB) {
            Serial.print("s d ");
            Serial.print(PICO_ID); Serial.print(" ");
            Serial.print(my()->u); Serial.print(" ");
            Serial.println(millis() - my()->initial_time);
        }
        else {
            if (my()->current_time - my()->last_time_stream >= 50) {
                uint8_t new_data[6] = {0};
                memcpy(new_data, &(my()->u), sizeof(my()->u));
                CanManager::enqueue_message(PICO_ID, my_type::SERIAL_STREAM_DUTY_CYCLE, new_data, sizeof(new_data));
                my()->last_time_stream = my()->current_time;
            }
        }
    }
}

void setup() {
    //Setup serial communication
    Serial.begin(115200);
    analogReadResolution(12);
    analogWriteFreq(30000); //30KHz
    analogWriteRange(4096); //Max PWM
    CanManager::flashIDsetup();
    
    //Setup controller, metrics and parser
    vars_setup();
    CanManager::createMap();
    delay(2000);
    Serial.println("Going inside wakeupgrid");
    CanManager::wake_up_grid();
    CanManager::printID();
    Serial.println("Going inside setupgains");

    distrControl::setUpGains();

    Serial.print("Vector of gains is: ");
    for (size_t i = 0; i < 4; ++i) {
            Serial.print(distrControl::gainsVector[i],4); // Print current element
            Serial.print(", "); // Print comma unless it's the last element
            }
    Serial.println(); 
    Serial.println("Going to loop");


}

void setup1() {
    CanManager::flashIDsetup();
    CanManager::begin(CAN_1000KBPS);
    //CanManager::setUpFiltersAndMasks();

}

void loop() {

    CanManager::serial_and_actions_rotine();
    controller_rotine();
    stream_rotine();
}

void loop1() {
    CanManager::canBusRotine();
}


