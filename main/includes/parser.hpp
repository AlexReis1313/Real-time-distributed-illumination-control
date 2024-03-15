#ifndef PARSER_HPP
#define PARSER_HPP

#include <Arduino.h>
#include <stdio.h>
#include <iostream>

class Parser {
public:
    Parser(float _reference, const int led_pin);
    Parser();
    ~Parser();
    Parser(const Parser& other);
    Parser& operator=(const Parser& other);
    void          parseCommand(const String& command);
    void          checkMenu(void);
    inline void   setReference(int i, float& val);
    float         getReference(int i); 

private:
    float&        reference;
    float         current_luminaire;
    int           led_pin;
    void          setDutyCycle(int& i, float& val);
    void          getDutyCycle(int& i);
};

#endif
 
