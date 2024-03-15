// Done by Duarte 13/03/2024

#include "includes/vars.h"

//k = 1500, tau = 0.263/10;

void setup() {
    vars_setup();
    time_vars_setup();
    analogReadResolution(12);
    analogWriteFreq(30000); //30KHz
    analogWriteRange(4095); //Max PWM
    Serial.begin();
}

void loop() {    
    if (Serial.available()) 
    {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Remove any whitespace
        my()->my_parser.parseCommand(command);
        my()->x_ref = my()->my_parser.getReference(0);
    }
    my()->ref_volts = LUX2Volt(my()->x_ref);
    my()->vss = analogRead(my()->LDR_port)*3.3/4095;
    //my()->vss = get_vss_non_blocking();
    if (my()->vss != -1)
    {
        if (millis() - time_vars()->last_control_time >= time_vars()->control_interval) {
            get_H_xref();
            get_H_x();
            my()->my_pid.setBcontroller((1 / (my()->H_xref * my()->gain * my()->k)) * 0.5);
            my()->vss_lux = Volt2LUX(my()->vss);
            Serial.print(my()->x_ref);
            Serial.print(" ");
            Serial.print(my()->vss_lux);
            Serial.print(" ");
            my()->u = my()->my_pid.compute_control(my()->ref_volts, my()->vss);
            Serial.print(my()->u * my()->gain + 0.02);
            Serial.println();
            analogWrite(my()->LED_PIN, (int)(my()->u));
            my()->my_pid.housekeep(my()->ref_volts, my()->vss);
            time_vars()->last_control_time = millis();
        }
    }
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
