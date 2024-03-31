#include "vars.h"
#include "vars_can_bus.h"
#include <iostream>

typedef void (*eventFunction)(info_msg &msg);

class CanManager {
    private:
        MCP2515 canController;
        unsigned char* interruptPin;
        canBus_vars canbus_vars;
        volatile bool dataAvailable;
        struct can_frame canMsgTx, canMsgRx;
        static std::map<int, eventFunction> _actionMap;
        static void ISR_wrapper();

    public:
        //Can Manager
        static CanManager *instance; // Static instance pointer for ISR access
        CanManager();
        void        begin(char bitrate);
        void        flashIDsetup();
        void        canInterrupt();
        void        setUpFiltersAndMasks ();
        bool        receiveMessage(struct can_frame &msg);
        bool        data_available();

        void        sendMessage1to0();
        static      info_msg extract_message(can_frame *frame);
        static      void enqueue_message(unsigned char* sender, my_type type, float data, unsigned char* size);

        //Can Actions
        void        createMap(void);
        static void offAction(info_msg &msg);
        static void onAction(info_msg &msg);
        static void ackAction(info_msg &msg);
        static void setDutyCycleAction(info_msg &msg);
        static void getDutyCycleAction(info_msg &msg);
        static void setReferenceAction(info_msg &msg);
        static void getReferenceAction(info_msg &msg);

};