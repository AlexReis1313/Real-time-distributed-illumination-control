// // Done by Duarte 13/03/2024
// #include "includes/vars.h"
// #include "includes/aux.h"
// #include "includes/vars_can_bus.h"

// void setup() {
//     Serial.begin(115200);
//     analogReadResolution(12);
//     analogWriteFreq(30000); //30KHz
//     analogWriteRange(4096); //Max PWM
//     vars_setup();
//     my()->my_pid.setBcontroller((1 / (my()->H_xref * my()->gain * my()->k)));    
// }

// void loop() {
//     if (!my()->inicial) {
//        Serial.begin(); Serial.print("x_ref"); Serial.print(" "); Serial.print("vss_lux"); Serial.print(" "); Serial.println("u*G"); my()->inicial = true;
//     }  
//     if (Serial.available()) 
//     {
//         String command = Serial.readStringUntil('\n');
//         command.trim(); // Remove any whitespace
//         my()->my_parser.parseCommand(command);
//         my()->x_ref = my()->my_parser.getReference(0);
//         my()->ref_volts = LUX2Volt(my()->x_ref); 
//     }
    
//     my()->current_time = millis();
//     if (my()->current_time - my()->last_control_time >= my()->control_interval) {
        
//         my()->vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
//         my()->vss_lux = Volt2LUX(my()->vss); //Get LDR value in lux

//         get_H_xref();
//         get_H_x();

//         my()->my_pid.setBcontroller((1 / (my()->H_xref * my()->gain * my()->k))); //Change b of controller based on the H_ref and H_x    
//         my()->u = my()->my_pid.compute_control(my()->ref_volts, my()->vss); // Compute control
//         analogWrite(my()->LED_PIN, (my()->u)); //Apply control signal to LED
//         my()->my_pid.housekeep(my()->ref_volts, my()->vss);

//         print_vars();
//         //float percnt_dutycycle = ((my()->vss * 4095) / 3.3) / 4095;
//         //my()->my_metrics.updateMetrics(my()->x_ref, my()->vss_lux , percnt_dutycycle);
//         my()->last_control_time = my()->current_time;
//     }
// }

// //cona

// //---------------------------------CAN BUS---------------------------------
// //the interrupt service routine
// // void read_interrupt(uint gpio, uint32_t events) {
// //     data_available = true; 
// //     //It simply sets a flag (data_available) to indicate 
// //     //that new data is available for processing.
// // }

// // void setup() {
// //    Serial.begin(115200);
// //    analogReadResolution(12);
// //    analogWriteFreq(30000); //30KHz
// //    analogWriteRange(4096); //Max PWM
// //    vars_setup();
// //    time_vars_setup();
// //    flash_get_unique_id(this_pico_flash_id); //get unique ID - node address
// //    node_address = this_pico_flash_id[7];
// //    //Serial.begin(); 
// //    can0.reset();
// //    can0.setBitrate(CAN_1000KBPS); //Setting bitrate
// //    can0.setNormalMode(); 
// //     gpio_set_irq_enabled_with_callback( //enabling interrupts for incoming messages
// //        interruptPin, GPIO_IRQ_EDGE_FALL,
// //        true, &read_interrupt );
// //     time_to_write = millis() + write_delay;
// // }
// // void loop() {
// //    if( millis() >= time_to_write ) {
// //        // Set up the CAN message to be sent
// //        canMsgTx.can_id = node_address; //CAN ID
// //        canMsgTx.can_dlc = 8; //data lenght: 8 bytes

// //        // Convert counterTx to a string of digits, then to ASCII values
// //        unsigned long div = counterTx*10;
// //        for( int i = 0; i < 8; i++ )
// //            canMsgTx.data[7-i]='0'+((div/=10)%10);

// //        // Send the CAN message
// //        err = can0.sendMessage(&canMsgTx);
// //        Serial.print("Sending message ");
// //        Serial.print( counterTx );
// //        Serial.print(" from node ");
// //        Serial.println( node_address, HEX );
// //        counterTx++;
// //        time_to_write = millis() + write_delay;
// //    }
// //    if( data_available ) {
// //        can0.readMessage(&canMsgRx);
// //        Serial.print("Received message number ");
// //        Serial.print( counterRx++ );
// //        Serial.print(" from node ");
// //        Serial.print( canMsgRx.can_id , HEX);
// //        Serial.print(" : ");
// //        for (int i=0 ; i < canMsgRx.can_dlc ; i++)
// //            Serial.print((char) canMsgRx.data[ i ]);
// //        Serial.println(" ");
// //        data_available = false;
// //    }
// // }
