#include "Arduino.h"
#include "includes/circularBuffer.hpp"

CircularBuffer::CircularBuffer(int _size) {
    this->size = _size;
    this->buffer_lux.reserve(this->size);
    this->buffer_duty_cycle.reserve(this->size);
}

CircularBuffer::~CircularBuffer() {
    this->buffer_lux.clear();
    this->buffer_duty_cycle.clear();
}

void CircularBuffer::addValueLux(float lux) {
    if (this->buffer_lux.size() == this->size) {
        this->buffer_lux.erase(this->buffer_lux.begin());
    }
    this->buffer_lux.push_back(lux);
}

void CircularBuffer::addValueDutyCycle(float duty_cycle) {
    if (this->buffer_duty_cycle.size() == this->size) {
        this->buffer_duty_cycle.erase(this->buffer_duty_cycle.begin());
    }
    this->buffer_duty_cycle.push_back(duty_cycle);
}

void CircularBuffer::printBufferLux() {
    for (int i = 0; i < this->buffer_lux.size(); i++) {
        Serial.println(this->buffer_lux[i]);
    }
}

void CircularBuffer::printBufferDutyCycle() {
    for (int i = 0; i < this->buffer_duty_cycle.size(); i++) {
        Serial.println(this->buffer_duty_cycle[i]);
    }
}

std::vector<float> CircularBuffer::getBufferLux() {
    return this->buffer_lux;
}

std::vector<float> CircularBuffer::getBufferDutyCycle() {
    return this->buffer_duty_cycle;
}