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
    }
    return m_median;
}

void get_gain(int value){ //value is a PWM of 3000
    float x_lux;
    float o; //voltage for zero light
    float x; //voltage for 3000 PWM
    float gain;

    delay(1500);
    analogWrite(my()->LED_PIN, 0);
    delay(3000);
    o = analogRead(my()->LDR_port)*3.3/4095;
    my()->o_lux = Volt2LUX(o);
    analogWrite(my()->LED_PIN, value);
    delay(3000);
    x = analogRead(my()->LDR_port)*3.3/4095;
    x_lux = Volt2LUX(x);
    my()->gain = (x_lux - my()->o_lux) / value;
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

void calculate_tau(int ledOff, int ledOn){
    int number_of_samples = 400;
    int i, current, my_time[number_of_samples];
    float vss, new_vss, voltage[number_of_samples];

    memset(voltage, 0, sizeof(float) * number_of_samples); //clear array
    memset(my_time, 0, sizeof(int) * number_of_samples);
    analogWrite(my()->LED_PIN, ledOff); // set inicial led PWM
    delay(1000);
    vss = analogRead(my()->LDR_port);
    delay(5000);
    analogWrite(my()->LED_PIN, ledOn); // set final led PWM
    int start_time = millis();
    while(i < number_of_samples)
    {
      new_vss = analogRead(my()->LDR_port)*3.3/4095;
      if(abs(vss - new_vss)/ new_vss <= 0.001f){ break; }
      voltage[i] = new_vss;
      my_time[i] = millis() - start_time;
      vss = new_vss;
      delay(20); 
      i++;
    }
    float voltage_tau = voltage[i-1] * 0.63;
    float tau;
    for(i = 0; i < number_of_samples; i++){
        if(voltage[i] > voltage_tau){
            tau = my_time[i];
            break;
        }
    }
    Serial.print("Tau is (s): "); Serial.println(tau / 1000, 30);
    delay(5000); //5s
}

void print_vars() {
    Serial.print(my()->x_ref); Serial.print(" ");
    Serial.print(my()->vss_lux); Serial.print(" ");
    Serial.println(my()->u * my()->gain + my()->o_lux);
}


