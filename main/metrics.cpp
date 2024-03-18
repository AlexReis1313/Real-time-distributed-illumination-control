#include "includes/metrics.hpp"

metrics::metrics () : visibility_error(0), \
                      average_flicker(0), flicker_sum(0), total_energy_consumption(0), \
                      visibility_sum(0), max_power(0), n_samples(0), duty_cicle_prev(0), \
                      luminance_prev(0), nomimal_power(0), instant_power(0), \
                      current_time(0), previous_time(0) {}

void metrics::setEnergyConsumption(float new_duty_cicle){
    current_time = millis();
    float delta_time = SAMPLE_TIME_MILIS * std::pow(10, -3) ;
    total_energy_consumption += nomimal_power * new_duty_cicle * delta_time;
    previous_time = current_time;

    instant_power = nomimal_power * new_duty_cicle;
    Serial.print("Energy consumption: "); Serial.println(total_energy_consumption);
}

//enviar referencias em LUX?
void metrics::setVisibilityError(float x_ref, float x_real){

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
    Serial.print("Visibility error: "); Serial.println(visibility_error);
}

void metrics::setAverageFlicker(float current_duty_cycle) {
    float delta_duty_cycle = abs(current_duty_cycle - duty_cicle_prev);
    if (delta_duty_cycle > 0) {
        flicker_sum += delta_duty_cycle;
    }
    duty_cicle_prev = current_duty_cycle;
    if (this->n_samples > 0) {
        average_flicker = flicker_sum / (this->n_samples);
    } else {
        average_flicker = 0;
    }
    Serial.print("Average Flicker: "); Serial.println(average_flicker);

}

float metrics::getEnergyConsumption(){
    return total_energy_consumption;
}

float metrics::getVisibilityError(){
    return visibility_error;
}

float metrics::getAverageFlicker(){
    return average_flicker;
}