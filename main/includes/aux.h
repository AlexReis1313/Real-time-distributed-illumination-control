// Done by Duarte 21/03/2024
#ifndef AUX_H
#define AUX_H

float       Volt2LUX(float v_in);
float       LUX2Volt(float LUX_in);
float       get_adc_digital_filter(const int n_size, int delay_Microseconds);
void        get_H_xref(); //x_ref is in lux
void        get_H_x();
void        get_gain(int value);
void        calculate_tau(int ledOff, int ledOn);

#endif