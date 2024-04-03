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
    CanManager::flashIDsetup();
    
    //Setup controller, metrics and parser
    vars_setup();
    //CanManager::wake_up_grid();
    // CanManager::wake_up_grid();
    CanManager::printID();
    CanManager::createMap();
    // distrControl::setUpGains();

}

void setup1() {
    CanManager::flashIDsetup();
    CanManager::begin(CAN_1000KBPS);
    //CanManager::setUpFiltersAndMasks();

}

void loop() {

    CanManager::serial_and_actions_rotine();
    controller_rotine();
}

void loop1() {
    CanManager::canBusRotine();
}


