#include <iostream>
#include <vector>

class CircularBuffer {

    private:
        std::vector<float> buffer_lux;
        std::vector<float> buffer_duty_cycle;
    public:
        CircularBuffer(int size);
        ~CircularBuffer();
        void addValueLux(float lux);
        void addValueDutyCycle(float duty_cycle);
        void printBufferLux();
        void printBufferDutyCycle();
        std::vector<float> getBufferLux();
        std::vector<float> getBufferDutyCycle();
        int size;
};