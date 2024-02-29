#ifndef PID_H
#define PID_H

class pid {
    private:
      float I, D, K, Ti, Td, b, h, y_old, N;
      
    public:
      //func(10); - without explicit
      //func(MyClass(10)); - with explicit
      explicit pid( float _h, float _K = 1, float b_ = 1,
                    float Ti_ = 1, float Td_ = 0, float N_ = 10);
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
    this->I += (this->K*this->h)/(this->Ti*e);
    this->y_old = y;
}

inline float saturate(float v, float ulow, float uhigh){
    if( v < ulow ) v = ulow;
    if( v > uhight ) v = uhigh;
    return v;
}

inline void pid::print_output(float u) {
    std::cout << "Control Output: " << u << std::endl;
}

#endif //PID_H
