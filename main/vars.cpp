#include "includes/vars.h"
#include "includes/aux.h"

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
    get_gain(3000);
    get_H_xref();
    get_H_x();
    my()->b_controller = 1;
    my()->u = 0;

    //Controller variables
    my()->my_pid = pid(0.01, my()->k, my()->b_controller, my()->tau, my()->Tt); 
    
    //Reference variables
    my()->x_ref = 10;
    my()->vss = analogRead(my()->LDR_port)*3.3/4095;
    my()->vss_lux = Volt2LUX(my()->vss);
    my()->ref_volts = LUX2Volt(my()->x_ref);
    //Parser variables
    my()->my_parser = Parser(my()->x_ref, my()->LED_PIN);

    //Metrics variables
    my()->my_metrics = Metrics();
    my()->inicial = false;
    my()->occupancy = true;

    my()->control_interval = 10;
}

// Can-bus setup
uint8_t         this_pico_flash_id[8], node_address;//node address(last byte of the flash ID)
struct can_frame canMsgTx, canMsgRx;
unsigned long   counterTx {0}, counterRx {0};
MCP2515::ERROR  err;
unsigned long   time_to_write;
unsigned long   write_delay {1000};
const byte      interruptPin {20};
volatile byte   data_available {false};
MCP2515         can0 {spi0, 17, 19, 16, 18, 10000000};
