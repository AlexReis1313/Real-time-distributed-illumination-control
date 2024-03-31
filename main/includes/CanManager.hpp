#include "vars.h"
#include "vars_can_bus.h"
#include <iostream>

typedef void (*eventFunction)(info_msg &msg);

class CanManager {
    private:
        static MCP2515 canController;
        static unsigned char* interruptPin;
        static volatile bool dataAvailable;
        static canBus_vars canbus_vars;
        static std::map<int, eventFunction> _actionMap;
        //struct can_frame canMsgTx, canMsgRx;
        //static void ISR_wrapper();
    public:
        //Can Manager
        static CanManager *instance; // Static instance pointer for ISR access
        static void     begin(char bitrate);
        static void     flashIDsetup();
        static void     canInterrupt();
        static void     setUpFiltersAndMasks ();
        static bool     receiveMessage(struct can_frame &msg);
        static bool     data_available();

        static void     sendMessage1to0();
        static info_msg extract_message(can_frame *frame);
        static void     enqueue_message(unsigned char sender, my_type type, unsigned char **message, std::size_t msg_size);
        static void     can_bus_rotine(void);

        //Can Actions
        static void     createMap(void);
        static void     offAction(info_msg &msg);
        static void     onAction(info_msg &msg);
        static void     ackAction(info_msg &msg);
        static void     setDutyCycleAction(info_msg &msg);
        static void     getDutyCycleAction(info_msg &msg);
        static void     setReferenceAction(info_msg &msg);
        static void     getReferenceAction(info_msg &msg);

};