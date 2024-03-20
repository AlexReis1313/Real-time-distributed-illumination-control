#include "includes/pid.h"

//Constructor
pid::pid() {}

pid::pid(float _h, float _K, float b_, float Ti_, float Tt_,
        float Td_, float N_): h {_h}, K {_K}, b {b_}, 
        Ti {Ti_}, Tt{Tt_} ,Td {Td_}, N {N_}, I {0.0}, D {0.0}, 
        y_old{0.0}, P{0.0}, u{0.0}, ao{0.0}, error{0.0} {
            this->feedforward = false; 
            this->antiwindup = true;
        } //b_old{0.0} Kold{0.0} {} // should check arguments validity

float pid::compute_control( float r, float y ) {

    //float bi = K*h/Ti; //integral gain, NEW
    //float ao = h/Tt; //backcalculation gain NEW Tt = Ti
    I = I + Kold*(b_old*r-y)-K*(b*r-y); // NEW update integral for bumpless transfer 
    Kold = K;
    b_old = b;    

    if (!feedforward) {
        P = K*(b*r-y); //proporcional part 
    } else {
        P = K*(b*r);
    } 
    u = this->saturate(P+I, 0, 4095); //saturate output
    return u;
}

void pid::setBcontroller (float b_controller){
    this->b = b_controller;
}
