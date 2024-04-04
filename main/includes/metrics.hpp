#include <iostream>
#include <cmath>
#include <Arduino.h>

#define SAMPLE_TIME_MILIS 10 //frequency 100Hz

class Metrics {
    public:
        Metrics();
        ~Metrics();
        void    setEnergyConsumption(float current_duty_cycle);
        float   getEnergyConsumption();
        void    setVisibilityError(float x_ref, float x_real);
        float   getVisibilityError();
        void    setMaxPower(float power);
        float   getMaxPower();
        void    setAverageFlicker(float current_duty_cycle);
        float   getAverageFlicker();
        float   getInstantPower();
        void    updateMetrics(float x_ref, float x_real, float new_duty_cycle);

    private:
        float   average_flicker;
        float   flicker_sum;

        float   total_energy_consumption;

        float   visibility_error;
        float   visibility_sum;

        float   max_power;
        int     n_samples;
        int     n_sample_flicker;
        float   duty_cicle_prev;
        float   luminance_prev;

        float   instant_power;

        float   current_time;
        float   previous_time;
};

