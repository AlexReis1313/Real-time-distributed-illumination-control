#include "vars.h"
#include "vars_can_bus.h"
#include <iostream>

typedef void (*eventFunction)(process_msg_t& msg);

class CanManager {
    private:
        MCP2515 canController;
        uint8_t interruptPin;
        canBus_vars canbus_vars;
        volatile bool dataAvailable;
        struct can_frame canMsgTx, canMsgRx;
        static std::map<int, eventFunction> _actionMap;
        static void ISR_wrapper();

    public:
        //Can Manager
        static CanManager *instance; // Static instance pointer for ISR access
        CanManager();
        void        begin(long bitrate);
        void        flashIDsetup();
        void        canInterrupt();
        void        setUpFiltersAndMasks ();
        bool        receiveMessage(struct can_frame &msg);
        bool        data_available();
        
        void        sendMessage1to0(can_bus *frame, can_bus **ptr_frame);
        void        enqueue_message(uint8_t sender, my_type type, uint8_t *message, std::size_t msg_size)

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