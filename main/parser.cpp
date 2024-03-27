#include "includes/vars.h"
#include "includes/parser.hpp"

float default_reference = 10.0;
Parser::Parser() : reference(default_reference), led_pin(0){
    this->current_luminaire = analogRead(15);
}
Parser::Parser(float _reference, const int _led_pin) : reference(_reference),
                                                        led_pin(_led_pin){
     this->current_luminaire = analogRead(_led_pin);
}

Parser& Parser::operator=(const Parser& other) {
    if (this != &other) {
        this->reference = other.reference;
        this->led_pin = other.led_pin;
    }
    return *this;
}

Parser::~Parser() {}

void Parser::parseCommand(const String& command) {
    char cmd = command.charAt(0); // Get the command type
    int i; //Led number pin
    float val;
    char item;
    if (command.length() == 0) {
        Serial.println("Command empty!");
        return;
    }
    switch (cmd) {
        case 'm':
            if (command.length() == 1){
              this->checkMenu();
              Serial.println("ack");
            }
            else {
               Serial.println("usage: 'm' ");
            }
            break;
        case 'g': // Set duty cycle
            if (sscanf(command.c_str(), "g %c %f", &item, &val) == 2){
                Serial.println("ack");
                this->getters(item); //val = duty_cycle
            }
            else{
                Serial.println("err");           
            }
            break;
        case 'r':
            if (sscanf(command.c_str(), "r %d %f", &i, &val) == 2){
                this->setReference(i, val);  
                Serial.println("ack");
            }
            else {
                Serial.println("err");
            }
            break;
        case 'd':
            if (sscanf(command.c_str(), "d %d %f", &i, &val) == 2){
                this->setDutyCycle(i, val);  
                Serial.println("ack");
            }
            else {
                Serial.println("err");
            }
            break;
        case 'o':
            if (sscanf(command.c_str(), "o %d %f", &i, &val) == 2){
                my()->occupancy = (bool)val;
                Serial.println("ack");
            }
            else {
                Serial.println("err");
            }
            break;
        case 'k': // Set feedback control
            if (sscanf(command.c_str(), "k %d %f", &i, &val) == 2){
                my()->my_pid.setFeedforward((bool)!val); 
                Serial.println("ack");
            }
            else {
                Serial.println("err");
            }
            break;
        case 'a':
            if (sscanf(command.c_str(), "a %d %f", &i, &val) == 2){
                my()->my_pid.setAntiWindup((bool)val);
                Serial.println("ack");
            }
            else {
                Serial.println("err");
            }
            break;
        default:
            Serial.println("err");
            break;
    }
}

void Parser::getters(char &item) {
    int i = 0;
    bool value;
    float result;
    int flag = 0;
    switch (item) {
        case 'd':
            result = this->getDutyCycle(i);
            break;
        case 'r':
            result = this->getReference(i);
            break;
        case 'o':
            result = my()->occupancy;
            break;
        case 'k':
            value = !my()->my_pid.getFeedforward();
            flag = 1;
            break;
        case 'a':
            value = my()->my_pid.getAntiWindup();
            flag = 1;
            break;
        case 'e':
            result = my()->my_metrics.getEnergyConsumption();
        case 'f':
            result = my()->my_metrics.getAverageFlicker();
        case 'v':
            result = my()->my_metrics.getVisibilityError();
        default:
            Serial.println("err");
            break;
    }
    Serial.print("Result: "); 
    if (flag == 1)
        Serial.println(value);
    else
        Serial.println(result);
    ;
}


void Parser::setDutyCycle(int &i, float &val) { //val (0, 4095)
    my()->my_pid.setDutyCycle(val, 10);
}

int Parser::getDutyCycle(int &i) {
    return my()->u;
}

inline void Parser::setReference(int i, float &val) {
    this->reference = val;
}

float Parser::getReference(int i) {
    return (this->reference);
}

void Parser::checkMenu(){
    Serial.println("Command Menu:");
    Serial.println("-------------");

    //Duty cycle
    Serial.println("Set directly the duty cycle of luminaire i:");
    Serial.println("'d <i> <val>' - Set directly the duty cycle of luminaire i to val");
    Serial.println("'g d <i>' - Show the duty cycle of the specified luminaire.");
 
    //Illuminance reference
    Serial.println("'r <i> <val>' - Set the illuminance reference of luminaire i to val in LUX");
    Serial.println("'g r <i>' - Show the illuminance reference of the specified luminaire.");
}