#include "includes/metrics.hpp"

Metrics::Metrics () : visibility_error(0), \
                      average_flicker(0), flicker_sum(0), total_energy_consumption(0), \
                      visibility_sum(0), max_power(0), n_samples(0), duty_cicle_prev(0), \
                      luminance_prev(0), nomimal_power(0), instant_power(0), \
                      current_time(0), previous_time(0), n_sample_flicker(0) {}

Metrics::~Metrics () {};

void Metrics::setEnergyConsumption(float current_duty_cycle){
    current_time = millis();
    float delta_time = SAMPLE_TIME_MILIS * std::pow(10, -3);
    nomimal_power = 0.017;
    instant_power = nomimal_power * current_duty_cycle;
    total_energy_consumption += instant_power * delta_time;
    previous_time = current_time;

    Serial.print("Energy consumption: "); Serial.println(total_energy_consumption, 10);
}


//enviar referencias em LUX?
void Metrics::setVisibilityError(float x_ref, float x_real){

    float visibility_sum;
    if (x_ref - x_real > 0) {
        visibility_sum += (x_ref - x_real);
        this->n_samples++;
    }
    if (this->n_samples > 0) {
        visibility_error = (visibility_sum / n_samples);
    }
    else {
        visibility_error = 0;
    }
    Serial.print("Visibility error: "); Serial.println(visibility_error, 10);
}

void Metrics::setAverageFlicker(float current_duty_cycle) {
    float delta_duty_cycle = abs(current_duty_cycle - duty_cicle_prev);
    //if is less than 1% of the previous duty cycle, it is considered flicker
    if (delta_duty_cycle > 0.01) {
        flicker_sum += delta_duty_cycle;
        this->n_sample_flicker++;
    }
    duty_cicle_prev = current_duty_cycle;
    if (this->n_sample_flicker > 0) {
        average_flicker = flicker_sum / (this->n_sample_flicker);
    } else {
        average_flicker = 0;
    }
    Serial.print("Average Flicker: "); Serial.println(average_flicker, 10);

}

void Metrics::updateMetrics(float x_ref, float x_real, float new_duty_cycle) {
    setEnergyConsumption(new_duty_cycle);
    setVisibilityError(x_ref, x_real);
    setAverageFlicker(new_duty_cycle);
}

float Metrics::getEnergyConsumption(){
    return this->total_energy_consumption;
}

float Metrics::getVisibilityError(){
    return this->visibility_error;
}

float Metrics::getAverageFlicker(){
    return this->average_flicker;
}
