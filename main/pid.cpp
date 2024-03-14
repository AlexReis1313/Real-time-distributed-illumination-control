#include "includes/pid.h"

//Constructor
pid::pid() {}

pid::pid(float _h, float _K, float b_, float Ti_, 
         float Td_, float N_): h {_h}, K {_K}, b {b_}, 
         Ti {Ti_}, Td {Td_}, N {N_}, I {800.0}, D {0.0}, 
         y_old{0.0} {} //b_old{0.0} Kold{0.0} {} // should check arguments validity

float pid::compute_control( float r, float y ) {

    float bi = K*h/Ti; //integral gain, NEW
    //float ad = Td/(Td+N*h); 
    //float bd = Td*K*N/(Td+N*h); //derivative gains
    float ao = h/Ti; //backcalculation gain NEW Tt = Ti
    I = I + Kold*(b_old*r-y)-K*(b*r-y); // NEW update integral for bumpless transfer 

    //Compute control
    float P = K*(b*r-y); //proporcional part  
    //D = ad*D-bd*(y-y_old); //update derivative part
    float temp_out = P+I;
    float u = this->saturate(temp_out, 0, 4095);
    //float u = temp_out;
    //this->print_output(u);
    I = I + bi*(r-y)+ao*(u-temp_out); //NEW
    y_old = y; //NEW
    Kold = K;
    b_old = b;    

    return u;
}

//a
