#include "pid.h"
#include <ostream>
#include <math.h>
#include <stdio.h>
#include "my_printf.hpp"
#include "parser.hpp"

pid       my_pid {0.1, 20, 1, 0.05 };
float     ref = 10.0f;
Parser    my_parser(ref, 15);

const int LED_PIN = 15;
const int DAC_RANGE = 4096;
int       counter = 0;
float     b = 5.3; //6.15
float     m = -0.8;
const int LDR_port = A0;
float     v_out = 0;
float     LUX = 0; //init lux
float     voltage[200];
int       my_time[200];
float     r_LDR = 0;


void setup() {
    analogReadResolution(12);
    analogWriteFreq(30000); //30KHz
    analogWriteRange(4095); //Max PWM
    Serial.begin();
}

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
    float volt = (3.3 * r_LDR) / (R1 + r_LDR);
    
    return volt;
}


void print_results(float v_in, float r_LDR){
  my_printf(Serial, " Lux: %f r_LDR: %f v_in: %f, v_out: %f", LUX, r_LDR, v_in, v_out); 
}

void loop() {
  if (Serial.available())
  {
      String command = Serial.readStringUntil('\n');
      Serial.read();
      command.trim(); // Remove any whitespace
      my_parser.parseCommand(command);
      ref = my_parser.getReference(0);
      //ref = Serial.parseInt();
      
      
  }
  Serial.print(ref);Serial.print(" ");
  //0 to 4095 - digital - valor digital
  float v_in = analogRead(LDR_port)*3.3/4095; //volts - analog - valor real
  float y = Volt2LUX(v_in);  
  Serial.print(y);Serial.print(" ");
  float u = my_pid.compute_control(ref, y);
  Serial.println();
  analogWrite(LED_PIN, u);
  my_pid.housekeep(ref, y);
  delay(100);
}

//void loop() {// the loop function runs cyclically
//    bool read = false;
//    memset(voltage, 0, sizeof(float) * 200);
//    memset(my_time, 0, sizeof(int) * 200);
//    float v_in = analogRead(LDR_port)*3.3/4095; //input voltage to 0-3.3V
//    float v_out = analogRead(LDR_port);
//    analogWrite(LED_PIN, 1); // set led PWM to 1
//    delay(1000);
//    v_out = analogRead(LDR_port);
//    delay(5000);
//    analogWrite(LED_PIN, 4000); // set led PWM to 4000
//    int start_time = millis();
//  
//    while((read == false) && i < 200)
//    {
//       float new_v_out = analogRead(LDR_port)*3.3/4095;
//       if(abs(v_out - new_v_out)/ new_v_out <= 0.000001f)
//          read = true;
//       voltage[i] = new_v_out;
//       int current = millis();
//       my_time[i] = current - start_time;
//       Serial.print(my_time[i]);Serial.print(" ");
//       Serial.print(v_out);
//       Serial.println();
//       v_out = new_v_out;
//       delay(25); //delay 1micro seconds
//       i++;
//    }
//    float voltage_tau = voltage[i-1] * 0.63;
//    float tau;
//    for(i = 0; i < 200; i++){
//      if(voltage[i] > voltage_tau){
//        tau = my_time[i];
//        break;
//      }
//    }
//    Serial.print("Tau is (s): "); Serial.print(tau / 1000, 10);
//    Serial.println();
//    delay(5000); //5s
//    calculate_LUX(v_in, &r_LDR);
//    print_results(v_in, r_LDR);
//}
