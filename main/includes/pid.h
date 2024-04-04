#ifndef PID_H
#define PID_H

#include "Arduino.h"

class pid {
    private:
      float I, D, K, Ti, Tt, Td, b, h, y_old, N, b_old, Kold;
      float P, u, error, ao, duty_cycle, dutycycle_time;
      bool  feedback, antiwindup;
      
    public:
      explicit pid( float _h, float _K = 0, float b_ = 1,
                    float Ti_ = 1, float Tt_ = 1,float Td_ = 0, float N_ = 0);
      pid(); //Default Constructor
      ~pid() {}; //Destructor
      float compute_control( float r, float y);
      void housekeep( float r, float y);  
      float saturate(float v, float ulow, float uhigh);
      void print_output(float u);
      void setBcontroller(float b_controller);
      bool getFeedback();
      void setFeedback(bool value);
      bool getAntiWindup();
      void setAntiWindup(bool value);
      void setDutyCycle(int duty_cycle, float time);
      float getDutyCycle();
      float getLastVss();
     
};

// inline means that it is expanded in code instead of 
// involving a functions call (thus more time efficient).
inline void pid::housekeep( float r, float y ) {
    error = r - y; //error
    if (antiwindup) {
        ao = h/Tt;
    } else {
        ao = 0;
    }
    float integral = 0;
    if (feedback) {
        integral = K*h/Ti*error;
    } 
    I += integral + ao*(u - (P+I));
    y = y_old;
}

inline float pid::saturate(float v, float ulow, float uhigh){
    if( v < ulow ) v = ulow;
    if( v > uhigh ) v = uhigh;
    return v;
}

inline void pid::print_output(float u) {
    Serial.print("Output: "); Serial.print(u);
    Serial.println();
}

#endif //PID_H
