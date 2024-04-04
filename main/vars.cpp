#include "includes/vars.h"
#include "includes/my_aux.h"

t_data *my(void){ //multithreading advantages; null inicialization; encapsulation, redability
    static t_data my_data;
    return (&my_data);
}

void vars_setup(void){    
    //Hardware variables
    my()->LED_PIN = 15;
    my()->LDR_port = A1;
    my()->DAC_RANGE = 4096;

    //LUX variables
    my()->b = 6.15;
    my()->m = -0.8;

    //Control variables //0, 10, 5
    my()->k = 250; //300; 750/50
    my()->tau = 0.19/10; //0.13 / 100
    my()->Tt = 1; //0.05;
    get_gain(3000);
    get_H_xref();
    get_H_x();
    my()->b_controller = 1;
    my()->u = 0;

    //Controller variables
    my()->my_pid = pid(0.01, my()->k, my()->b_controller, my()->tau, my()->Tt); 
    my()->my_pid.setBcontroller((1 / (my()->H_xref * my()->gain * my()->k)));    
    
    //Reference variables
    my()->x_ref = 10;
    my()->vss = analogRead(my()->LDR_port)*3.3/4095;
    my()->vss_lux = Volt2LUX(my()->vss);
    my()->ref_volts = LUX2Volt(my()->x_ref);
    my()->o_lux = -1;
    //Parser variables
    my()->my_parser = Parser();

    //Metrics variables
    my()->my_metrics = Metrics();
    my()->inicial = false;
    my()->occupancy = false;
    my()->nominal_power = 0.021;

    //Stream variables
    my()->stream_lux = false;
    my()->stream_duty_cycle = false;

    //Time variables
    my()->control_interval = 10;
    my()->initial_time = millis();

    // Initialize list_Nr_detected_IDS and list_IDS
    my()->list_Nr_detected_IDS.clear(); // Ensure the list is empty initially
    my()->list_IDS.clear(); // Ensure the list is empty initially
    my()->list_nodes.clear();

    float percnt_dutycycle = (my()->u) / 4095;
    my()->my_metrics.updateMetrics(my()->x_ref, my()->vss_lux , percnt_dutycycle);
}
