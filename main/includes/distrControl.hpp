//#include "vars_can_bus.h"
#include <iostream>



class distrControl {
    private:
        distrControl() {}
    public:
        static distrControl* instance;
        
        static bool endGAINS_bool;
        
        static float gainsVector[4];
       
        static void  setUpGains();
        

};