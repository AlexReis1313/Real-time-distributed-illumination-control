#include "includes/aux.h"
#include "includes/vars.h"


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

float get_adc_digital_filter(const int n_size, int delay_Microseconds) {
    float arr[n_size];
    int mid_index;
    float m_median;
    for (size_t i = 0; i < n_size; i++) { // 10 microseconds delay between measurements
        arr[i] = analogRead(my()->LDR_port);
        delayMicroseconds(delay_Microseconds);
    }
    std::sort(arr, arr + n_size); 
    mid_index = n_size / 2;
    if (!(n_size % 2)) {
        m_median = (arr[mid_index - 1] + arr[mid_index]) / 2.0;
    } else {
        m_median = arr[int(mid_index)];
    }    //Serial.print("H_xref: "); Serial.println(H_xref, 10);
    return m_median;
}

void get_gain(void){
    float x_lux;
    float o; //voltage for zero light
    float x; //voltage for 3000 PWM
    float gain;

    delay(1500);
    analogWrite(my()->LED_PIN, 0);
    delay(3000);
    o = analogRead(my()->LDR_port)*3.3/4095;
    my()->o_lux = Volt2LUX(o);
    analogWrite(my()->LED_PIN, 3000);
    delay(3000);
    x = analogRead(my()->LDR_port)*3.3/4095;
    x_lux = Volt2LUX(x);
    my()->gain = (x_lux - my()->o_lux) / 3000;
    Serial.print("Gain: "); Serial.println(my()->gain, 10);
    delay(1000);
}

//x_ref is in lux
void get_H_xref(void){ 
    my()->ref_volts = LUX2Volt(my()->x_ref);
    my()->H_xref = (my()->ref_volts / my()->x_ref);
}

void get_H_x(void){
    my()->vss_lux = Volt2LUX(my()->vss);
    my()->H_x = (my()->vss / my()->vss_lux);
}

//0.16 -> 1000
//0.30 -> 3000
//0.263 -> 4000
void calculate_tau(int i = 0){
    float           voltage[400];
    int             my_time[400];
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
