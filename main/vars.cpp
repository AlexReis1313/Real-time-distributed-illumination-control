#include "includes/vars.h"
#include "includes/my_aux.h"

t_data *my(void){ //multithreading advantages; null inicialization; encapsulation, redability
    static t_data my_data;
    return (&my_data);
}

void vars_setup(void){    
    //Hardware variables
    my()->LED_PIN = 15;
    my()->LDR_port = A0;
    my()->DAC_RANGE = 4096;

    //LUX variables
    my()->b = 6.15;
    my()->m = -0.8;

    //Control variables //0, 10, 5
    my()->k = 250; //300; 750/50
    my()->tau = 0.19/10; //0.13 / 100
    my()->Tt = 1; //0.05;
    //get_gain(3000);
    my()->gain = 0.01 //this will be changed after calculating gains for all nodes - it is just an initialization
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
    my()->last_sent_consensus = millis();
    // Initialize list_Nr_detected_IDS and list_IDS
    my()->list_Nr_detected_IDS.clear(); // Ensure the list is empty initially
    my()->list_IDS.clear(); // Ensure the list is empty initially
    my()->list_nodes.clear();

    float percnt_dutycycle = (my()->u) / 4095;
    my()->my_metrics.updateMetrics(my()->x_ref, my()->vss_lux , percnt_dutycycle);


    //initialize consensus
    my()->wait_change_iter = false;
    my()->last_consensus_time=millis();
    my()->first_loop = 100;
    my()->sendingConsensus_begin = false;
    my()->consensus_iteration = 0;
    my()->consensus_maxIterations = 20;
    my()->consensus_ongoing = false;
    //my()->send_consensus = false;
    my()->list_Nr_detected_consensus.clear(); //contins how many consensus the others have receive. When all have received nr_ckechIn_Nodes - 1, then move to next iter
    my()->list_consesus_received_vector.clear();


    my()->list_stream_lux = std::vector<bool>(my()->nr_ckechIn_Nodes, false);
    my()->list_stream_duty_cycle = std::vector<bool>(my()->nr_ckechIn_Nodes, false);
    my()->list_stream_last_minute_lux = std::vector<bool>(my()->nr_ckechIn_Nodes, false);
    my()->list_stream_last_minute_duty_cycle = std::vector<bool>(my()->nr_ckechIn_Nodes, false);

    //create pico buffers without allocation
    std::vector<CircularBuffer> pico_buffers(my()->nr_ckechIn_Nodes, CircularBuffer((60 * 1000) / 50));
    my()->pico_buffers = pico_buffers;


}
