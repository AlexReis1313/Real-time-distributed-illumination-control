#ifndef PARSER_HPP
#define PARSER_HPP

#include <Arduino.h>
#include <stdio.h>
#include <iostream>

class Parser {
public:
    Parser(float& _reference, const int led_pin);
    ~Parser();
    void          parseCommand(const String& command);
    void          checkMenu(void);
    inline void   setReference(int i, float& val);
    float         getReference(int i); 

private:
    float&        reference;
    float         current_luminaire;
    const int     led_pin;
    void          setDutyCycle(int& i, float& val);
    void          getDutyCycle(int& i);
};

#endif
 
