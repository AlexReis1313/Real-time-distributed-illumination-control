#include "parser.hpp"

//Command Menu:
//-------------
//Set directly the duty cycle of luminaire i:
//' d <i> <val> ' - Set directly the duty cycle of luminaire i to val
//' r <i> <val> ' - Set the illuminance reference of luminaire i to val in LUX
//' g  d   <i>  ' - Show the duty cycle of the specified luminaire.
//' g  r   <i>  ' - Show the illuminance reference of the specified luminaire.

Parser::Parser(float &_reference, const int _led_pin) : reference(_reference),
                                                        led_pin(_led_pin){
     this->current_luminaire = analogRead(_led_pin);
}

void Parser::parseCommand(const String& command) {
    char cmd = command.charAt(0); // Get the command type
    int i; //Led number pin
    float val;
    if (command.length() == 0) {
        Serial.println("Command empty!");
        return;
    }
    switch (cmd) {
        case 'm':
            if (command.length() == 1)
              this->checkMenu();
            else
               Serial.println("usage: 'm' ");
            break;
        case 'd': // Set duty cycle
            if (sscanf(command.c_str(), "d %d %f", &i, &val) == 2){
                this->setDutyCycle(i, val); //val = duty_cycle
            }
            else{
                Serial.println("err");           
            }
            break;
        case 'g':
            if (command.charAt(2) == 'd' && \
                sscanf(command.c_str(), "g d %d", &i) == 1){
                this->getDutyCycle(i);
            }
            else if (command.charAt(2) == 'r' && \
                sscanf(command.c_str(), "g r %d", &i) == 1){
                this->getReference(i);
            }
            else {
                Serial.println("err");
            }
            break;
        case 'r':
            if (sscanf(command.c_str(), "r %d %f", &i, &val) == 2){
                this->setReference(i, val);   
            }
            else {
                Serial.println("err");
            }
            break;
//        default:
//            Serial.println("err");
//            break;
    }
}

//Usage: d <i> <val> :Set directly the duty cycle of luminaire i 
//<val> is a number that reflects the duty cycle
void Parser::setDutyCycle(int &i, float &val) { //val (0, 4095)
    analogWrite(this->led_pin, static_cast<int>(val));
    //Serial.println("ack");
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
//Usage: g d <i>
void Parser::getDutyCycle(int &i) {
    int dutyCycle = analogRead(this->led_pin); // Read the current PWM value or a stored value that reflects the duty cycle
    // Serial.print("d ");
    // Serial.print(i);
    // Serial.print(" ");
    // Serial.println(dutyCycle);
}

inline void Parser::setReference(int i, float &val) {
    this->reference = val;
    //std::cout << "ack" << std::endl;
}

float Parser::getReference(int i) {
    //int r = this->reference;
    //Serial.print("r ");
    //Serial.print(i);
    //Serial.print(" ");
    //Serial.println(r);
    return (this->reference);
}
