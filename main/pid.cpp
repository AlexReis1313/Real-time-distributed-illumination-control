#include "pid.h"

//Constructor
pid::pid(float _h, float _K, float b_, float Ti_, 
         float Td_, float N_): h {_h}, K {_K}, b {b_}, 
         Ti {Ti_}, Td {Td_}, N {N_}, I {0.0}, D {0.0}, 
         y_old{0.0}{ } // should check arguments validity

float pid::compute_control( float r, float y ) {

    //Compute controller coefficients
    //float bi = K*h/Ti; //integral gain, NEW
    float ad = Td/(Td+N*h); 
    float bd = Td*K*N/(Td+N*h); //derivative gains
    //float ao = h/Tt; //backcalculation gain NEW
    //I = I + Kold*(bold*r-y)-K*(b*r-y); // NEW update integral for bumpless transfer 

    //Read inputs
    r = analogRead(LED_PIN);
    y = analogRead(A0);

    //Compute control
    float P = K*(b*r-y); //proporcional part  
    D = ad*D-bd*(y-y_old); //update derivative part
    float temp_out = P+I+D;
    float u = sat(temp_out, 0, 4095);
    this->print_output(u);
    //I = I + bi*(r-y)+ao*(u-v); //NEW
    yold = y; //NEW
    return u;
}
