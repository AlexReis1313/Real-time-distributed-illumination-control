#include "includes/pid.h"
#include "includes/vars.h"

//Constructor
pid::pid() {}

pid::pid(float _h, float _K, float b_, float Ti_, float Tt_,
        float Td_, float N_): h {_h}, K {_K}, b {b_}, 
        Ti {Ti_}, Tt{Tt_} ,Td {Td_}, N {N_}, I {0.0}, D {0.0}, 
        P{0.0}, u{0.0}, ao{0.0}, error{0.0}, b_old{0.0}, 
        Kold{0.0}, y_old{0.0}{
            setFeedforward(false);
            setWindup(true);
        }

float pid::compute_control( float r, float y ) {
    I = I + Kold*(b_old*r-y)-K*(b*r-y); // Update the integral for bumpless transfer 
    Kold = K;
    b_old = b;    

    if (!feedforward) {
        P = K*(b*r-y); //proporcional part 
    } else {
        P = K*(b*r);
    } 
    u = this->saturate(P+I, 0, my()->DAC_RANGE); //saturate output
    return u;
}

//Setters
void pid::setFeedforward(bool value) {
    this->feedforward = value;
}

void pid::setWindup(bool value) {
    this->antiwindup = value;
}
void pid::setBcontroller (float b_controller){
    this->b = b_controller;
}
