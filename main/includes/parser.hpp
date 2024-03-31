#ifndef PARSER_HPP
#define PARSER_HPP

#include <Arduino.h>
#include <stdio.h>
#include <iostream>

class Parser {
public:
    Parser();
    ~Parser();
    void          parseCommand(const String& command);
    void          checkMenu(void);
    inline void   setReference(int i, float& val);
    float         getReference(int i); 
    void          getters(char &item);
};
#endif
 
