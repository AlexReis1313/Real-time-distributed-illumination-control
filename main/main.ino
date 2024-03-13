#include "pid.h"
#include <ostream>
#include <math.h>
#include <stdio.h>
#include "my_printf.hpp"
#include "parser.hpp"
#include <hardware/flash.h> //for flash_get_unique_id
#include "mcp2515.h"
#include "vars.h"

void setup() {
    analogReadResolution(12);
    analogWriteFreq(30000); //30KHz
    analogWriteRange(4095); //Max PWM
    Serial.begin();
}

double get_gain(){
    float o_lux;
    float x_lux;
    float o; //voltage for zero light
    float x; //voltage for 3000 PWM
    float gain;

    analogWrite(LED_PIN, 0);
    delay(3000);
    o = analogRead(LDR_port)*3.3/4095;
    o_lux = Volt2LUX(o);
    delay(1000);
    //Serial.print("o: "); Serial.println(o);

    analogWrite(LED_PIN, 3000);
    delay(3000);
    x = analogRead(LDR_port)*3.3/4095;
    x_lux = Volt2LUX(x);
    //Serial.print("x: "); Serial.println(x_lux);

    gain = (x_lux - o_lux) / 3000;
    Serial.print("Gain: "); Serial.println(gain, 10);
    delay(1000);
    return gain;
}

//x_ref is in lux
double get_H_xref(float x_ref){
    float volt_ref;
    float H_xref;

    volt_ref = LUX2Volt(x_ref);
    H_xref = (volt_ref / x_ref);
    Serial.print("H_xref: "); Serial.println(H_xref, 10);
    return (H_xref);
}

double get_H_x(float x_ref, float vss){
    float H_x;
    float vss_lux;

    vss_lux = Volt2LUX(vss);
    H_x = (vss / vss_lux);
    Serial.print("H_x: "); Serial.println(H_x, 10);
    return (H_x);
}

float vss;
float y;
float ref_volts;
float vss_lux;
float G ; //0.0041873786
float H_xref; //0.0751975700 para ref=10
float H_x; //0.0592132099 para ref=10
bool gain_setup = false;
void loop() {
    //Serial.print("HI"); Serial.println();
    //calculate_tau(voltage, my_time);
    if (gain_setup == false){
        G = get_gain();
        gain_setup = true;
    }
    H_xref = get_H_xref(x_ref);
    H_x = get_H_x(x_ref, vss);
    if (Serial.available()) 
    {
      String command = Serial.readStringUntil('\n');
      command.trim(); // Remove any whitespace
      my_parser.parseCommand(command);
      x_ref = my_parser.getReference(0);
    }
    ref_volts = LUX2Volt(x_ref);
    Serial.print(x_ref);Serial.print(" ");//0 to 4095 - digital - valor digital
    vss = analogRead(LDR_port)*3.3/4095; //volts - analog - valor real
    vss_lux = Volt2LUX(vss);
    Serial.print(vss_lux);Serial.print(" ");
    float u = my_pid.compute_control(ref_volts, vss);
    Serial.println();
    analogWrite(LED_PIN, (int)u);
    my_pid.housekeep(x_ref, y);
    delay(100);
}


//---------------------------------CAN BUS---------------------------------
//the interrupt service routine
void read_interrupt(uint gpio, uint32_t events) {
    data_available = true; 
    //It simply sets a flag (data_available) to indicate 
    //that new data is available for processing.
}

// void setup() {
//    flash_get_unique_id(this_pico_flash_id); //get unique ID - node address
//    node_address = this_pico_flash_id[7];
//    Serial.begin(); 
//    can0.reset();
//    can0.setBitrate(CAN_1000KBPS); //Setting bitrate
//    can0.setNormalMode(); 
//     gpio_set_irq_enabled_with_callback( //enabling interrupts for incoming messages
//        interruptPin, GPIO_IRQ_EDGE_FALL,
//        true, &read_interrupt );
//     time_to_write = millis() + write_delay;
// }
// void loop() {
//    if( millis() >= time_to_write ) {
//        // Set up the CAN message to be sent
//        canMsgTx.can_id = node_address; //CAN ID
//        canMsgTx.can_dlc = 8; //data lenght: 8 bytes

//        // Convert counterTx to a string of digits, then to ASCII values
//        unsigned long div = counterTx*10;
//        for( int i = 0; i < 8; i++ )
//            canMsgTx.data[7-i]='0'+((div/=10)%10);

//        // Send the CAN message
//        err = can0.sendMessage(&canMsgTx);
//        Serial.print("Sending message ");
//        Serial.print( counterTx );
//        Serial.print(" from node ");
//        Serial.println( node_address, HEX );
//        counterTx++;
//        time_to_write = millis() + write_delay;
//    }
//    if( data_available ) {
//        can0.readMessage(&canMsgRx);
//        Serial.print("Received message number ");
//        Serial.print( counterRx++ );
//        Serial.print(" from node ");
//        Serial.print( canMsgRx.can_id , HEX);
//        Serial.print(" : ");
//        for (int i=0 ; i < canMsgRx.can_dlc ; i++)
//            Serial.print((char) canMsgRx.data[ i ]);
//        Serial.println(" ");
//        data_available = false;
//    }
// }

