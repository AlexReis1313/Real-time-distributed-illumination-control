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
    void          getters(char &item, int val);
    void          getBuffer(char x, int i);
};
#endif
 
