#include "includes/vars.h"

t_data *my(void){ //multithreading advantages; null inicialization; encapsulation, redability
    static t_data my_data;

    return (&my_data);
}

void vars_setup(void){
    //Control variables //0, 10, 5
    my()->k = 300;
    my()->tau = 0.263/10;
    my()->b_factor = 5; //more b 
    my()->Tt = 0.05;
    //get_H_x();
    my()->b_controller = (1 / (my()->H_xref * my()->gain * my()->k))* my()->b_factor;

    //LUX variables
    my()->b = 6.15;
    my()->m = -0.8;

    //Hardware variables
    my()->LED_PIN = 15;
    my()->LDR_port = A0;
    my()->DAC_RANGE = 4096;

    //Reference variables
    my()->x_ref = 10;

    //Parser variables
    my()->my_parser = Parser(my()->x_ref, my()->LED_PIN);

    //Controller variables
    my()->my_pid = pid(0.01, my()->k, my()->b_controller, my()->tau, my()->Tt); 
    //my()->my_pid = pid(0.01, 20, 1, 0.05);
    get_gain();
    get_H_xref();
}

t_time_vars *time_vars(void){ //multithreading advantages; null inicialization; encapsulation, redability
    static t_time_vars my_time_data;

    return (&my_time_data);
};


void time_vars_setup(void){
    time_vars()->n_measurements = 10;
    time_vars()->measurement_interval = 10; // Time between measurements in milliseconds
    time_vars()->measurement_complete = false;
    time_vars()->control_interval = 10;
   
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

float           voltage[400];
int             my_time[400];

float Volt2LUX(float v_in){
    float R1 = 10000;
    float r_LDR = R1 * (3.3 - v_in)/v_in; //resistance of LDR
    float LUX = pow(10, ((log10(r_LDR) - my()->b) / my()->m));
    return LUX;
}

float LUX2Volt(float LUX_in){
    float log_lux = log10(LUX_in);
    float R1 = 10000;
    float r_LDR = pow(10, (log_lux * my()->m + my()->b));
    float volt = (3.3 * R1) / (R1 + r_LDR);
    
    return volt;
}

//0.16 -> 1000
//0.30 -> 3000
//0.263 -> 4000
void calculate_tau(float voltage[], int my_time[], int i = 0){
   float vss;
   bool read = false;
   float new_vss;
   int current;

   memset(voltage, 0, sizeof(float) * 400); //clear array
   memset(my_time, 0, sizeof(int) * 400);
   analogWrite(my()->LED_PIN, 0); // set led PWM to 1
   delay(1000);
   vss = analogRead(my()->LDR_port);
   delay(5000);
   analogWrite(my()->LED_PIN, 4000); // set led PWM to 4000
   int start_time = millis();
 
   while((read == false) && i < 400)
   {
      new_vss = analogRead(my()->LDR_port)*3.3/4095;
      if(abs(vss - new_vss)/ new_vss <= 0.000001f)
         read = true;
      voltage[i] = new_vss;
      current = millis();
      my_time[i] = current - start_time;
      Serial.print(my_time[i]);Serial.print(" ");
      Serial.print(vss);
      Serial.println();
      vss = new_vss;
      delay(20); //delay 1micro seconds,more precise with less time
      i++;
   }
   float voltage_tau = voltage[i-1] * 0.63;
   float tau;
   for(i = 0; i < 400; i++){
     if(voltage[i] > voltage_tau){
       tau = my_time[i];
       break;
     }
   }
   Serial.print("Tau is (s): "); Serial.print(tau / 1000, 10);
   Serial.println();
   delay(5000); //5s
}

void get_gain(void){
    float o_lux;
    float x_lux;
    float o; //voltage for zero light
    float x; //voltage for 3000 PWM
    float gain;

    analogWrite(my()->LED_PIN, 0);
    delay(3000);
    o = analogRead(my()->LDR_port)*3.3/4095;
    o_lux = Volt2LUX(o);
    delay(1000);
    //Serial.print("o: "); Serial.println(o);

    analogWrite(my()->LED_PIN, 3000);
    delay(3000);
    x = analogRead(my()->LDR_port)*3.3/4095;
    x_lux = Volt2LUX(x);
    //Serial.print("x: "); Serial.println(x_lux);

    my()->gain = (x_lux - o_lux) / 3000;
    //Serial.print("Gain: "); Serial.println(my()->gain, 10);
    delay(1000);
}

//x_ref is in lux
void get_H_xref(void){ 
    my()->ref_volts = LUX2Volt(my()->x_ref);
    my()->H_xref = (my()->ref_volts / my()->x_ref);
    //Serial.print("H_xref: "); Serial.println(H_xref, 10);
}

void get_H_x(void){
    my()->vss_lux = Volt2LUX(my()->vss);
    my()->H_x = (my()->vss / my()->vss_lux);
    //Serial.print("H_x: "); Serial.println(H_x, 10);
}

// std::vector<float> measurements;
// bool measurement_complete = false;
// int measurement_index = 0;
// int mid_idx;
// float median;
// unsigned long last_measurement_time = 0;
// int n_measurements = 10; // replace with your value
// int measurement_interval = 10; // replace with your value

// int get_vss_non_blocking(){ //digital filtering
//     measurements.resize(n_measurements);

//     if (measurement_complete) {
//         // Reset the state for the next measurement
//         measurement_complete = false;
//         measurement_index = 0;

//         // Calculate median
//         std::sort(measurements.begin(), measurements.end());

//         mid_idx = n_measurements / 2;
//         if (n_measurements % 2 == 0) {
//             median = (measurements[mid_idx - 1] + measurements[mid_idx]) / 2.0;
//         } else {
//             median = measurements[mid_idx];
//         }
//         my()->vss = median * 3.3 / 4095; // Convert to volts
//     }
//     else if (measurement_index < n_measurements) {
//         if (millis() - last_measurement_time >= measurement_interval) {
//             measurements[measurement_index++] = analogRead(my()->LDR_port);
//             last_measurement_time = millis();

//             if (measurement_index == n_measurements) {
//                 measurement_complete = true; // Signal that the measurements are complete
//             }
//         }
//     }
//     return -1; // Indicate that measurement is not complete
// }
// int get_vss_non_blocking(){ //digital filtering
//     static std::vector<float> measurements(time_vars()->n_measurements);

//     if (time_vars()->measurement_complete) {
//         // Reset the state for the next measurement
//         time_vars()->measurement_complete = false;
//         time_vars()->measurement_index = 0;

//         // Calculate median
//         std::sort(measurements.begin(), measurements.end());

//         time_vars()->mid_idx = time_vars()->n_measurements / 2;
//         if (time_vars()->n_measurements % 2 == 0) {
//             time_vars()->median = (measurements[time_vars()->mid_idx - 1] \
//                                   + measurements[time_vars()->mid_idx]) / 2.0;
//         } else {
//             time_vars()->median = measurements[time_vars()->mid_idx];
//         }
//         my()->vss = time_vars()->median * 3.3 / 4095; // Convert to volts
//     }
//     else if (time_vars()->measurement_index < time_vars()->n_measurements) {
//         if (millis() - time_vars()->last_measurement_time >= time_vars()->measurement_interval) {
//             measurements[time_vars()->measurement_index++] = analogRead(my()->LDR_port);
//             time_vars()->last_measurement_time = millis();

//             if (time_vars()->measurement_index == time_vars()->n_measurements) {
//                 time_vars()->measurement_complete = true; // Signal that the measurements are complete
//             }
//         }
//     }
//     return -1; // Indicate that measurement is not complete
// }
