#ifndef PID_H
#define PID_H
#include <stdio.h>
#include <Arduino.h>

class pid {
    private:
      float I, D, K, Ti, Td, b, h, y_old, N, b_old, Kold;
      float LED_PIN = 16;
      
    public:
      //func(10); - without explicit
      //func(MyClass(10)); - with explicit
      explicit pid( float _h, float _K = 0, float b_ = 1,
                    float Ti_ = 1, float Td_ = 0, float N_ = 10);
      pid(); //Default Constructor
      ~pid() {}; //Destructor
      float compute_control( float r, float y);
      void housekeep( float r, float y);
      float saturate(float v, float ulow, float uhigh);
      void print_output(float u);
};

// inline means that it is expanded in code instead of 
// involving a functions call (thus more time efficient).
inline void pid::housekeep( float r, float y ) {
    float e;
    
    e = r - y; //error
    I += (K*h)/Ti*e;
    y_old = y;
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
