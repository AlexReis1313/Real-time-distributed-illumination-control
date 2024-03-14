#include "includes/vars.h"

const int       LED_PIN = 15;
const int       DAC_RANGE = 4096;
const float     b = 6.15; //6.15 ; 5.33
const float     m = -0.8;
const int       LDR_port = A0;
float           x_ref = 10.0f;

float           vss = 0.0; // Initialize with default values as appropriate
float           ref_volts = 0.0;
float           vss_lux = 0.0;
float           G = 0.0; // Initialize with the value if known, or 0 otherwise
float           H_xref = 0.0;
float           H_x = 0.0;
bool            gain_setup = false;
float           bk = 0.0;
float           b_controller = 0.0;

//pid             my_pid(0.1, 10, 1, 0.263, 0, 0);
Parser          my_parser(x_ref, LED_PIN);

// Can-bus setup
uint8_t         this_pico_flash_id[8], node_address; //node address(last byte of the flash ID)
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
    float LUX = 0; //init lux
    
    LUX = pow(10, ((log10(r_LDR) - b) / m));
    return LUX;
}

float LUX2Volt(float LUX_in){
    float log_lux = log10(LUX_in);
    float R1 = 10000;
    float r_LDR = pow(10, (log_lux * m + b));
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
   analogWrite(LED_PIN, 0); // set led PWM to 1
   delay(1000);
   vss = analogRead(LDR_port);
   delay(5000);
   analogWrite(LED_PIN, 4000); // set led PWM to 4000
   int start_time = millis();
 
   while((read == false) && i < 400)
   {
      new_vss = analogRead(LDR_port)*3.3/4095;
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

