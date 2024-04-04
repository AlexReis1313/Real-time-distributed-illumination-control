#include "vars.h"
#include "vars_can_bus.h"
#include <iostream>

typedef void (*eventFunction)(info_msg &msg);

#define PICO_ID CanManager::canbus_vars.node_address
#define HUB CanManager::hub

class CanManager {
    private:
        static MCP2515 canController;
        static unsigned char* interruptPin;
        static volatile bool dataAvailable;
        static std::map<my_type, eventFunction> _actionMap;
        //struct can_frame canMsgTx, canMsgRx;
        //static void ISR_wrapper();
    public:
        static canBus_vars canbus_vars;
        static unsigned char hub;
        static bool hubFlag;
        static bool hubFound;
        //Can Manager
        static CanManager *instance; // Static instance pointer for ISR access
        static void     begin(char bitrate);
        static void     flashIDsetup();
        static void     canInterrupt();
        static void     setUpFiltersAndMasks ();
        static bool     receiveMessage(struct can_frame &msg);
        static bool     data_available();

        static void     canBusRotine();
        static void     canBUS_to_actions_rotine(bool executeAction);
        static info_msg extract_message(can_frame *frame);
        static void     enqueue_message(unsigned char sender, my_type type, unsigned char *message, std::size_t msg_size);
        static void     serial_and_actions_rotine(void);
        static void     checkHub();
        static void     printID();
        static void     wake_up_grid();
        static bool     check_wake_up_condition();
        static void     acknoledge(char type,unsigned char data );
        static void     loopUntilACK(int nrOfAcknoledge, unsigned char sender, my_type type, unsigned char *message, std::size_t msg_size);


        //Can Actions
        static void     createMap(void);

        //Ack
        static void     ackAction(info_msg &msg);

        //SETTERS
        static void     setReferenceAction(info_msg &msg);
        static void     setDutyCycleAction(info_msg &msg);
        static void     setOccupancyAction(info_msg &msg);
        static void     setAntiWindupAction(info_msg &msg);
        static void     setFeedbackAction(info_msg &msg);

        //STREAM
        // static void     startStreamLuxAction(info_msg &msg);
        // static void     startStreamDutyCycleAction(info_msg &msg);
        // static void     stopStreamDutyCycleAction(info_msg &msg);
        // static void     stopStreamLuxAction(info_msg &msg);
        // static void     startBufferPrintLuxAction(info_msg &msg);
        // static void     startBufferPrintDutyCycleAction(info_msg &msg);

        //GETTERS
        static void     getDutyCycleAction(info_msg &msg);
        static void     getReferenceAction(info_msg &msg);
        static void     getIluminanceAction(info_msg &msg);
        static void     getOccupancyAction(info_msg &msg);
        static void     getAntiWindupAction(info_msg &msg);
        static void     getFeedbackAction(info_msg &msg);
        static void     getExternalIluminanceAction(info_msg &msg);
        static void     getElapsedTimeAction(info_msg &msg);
        static void     getInstantaneousPowerAction(info_msg &msg);
        static void     getAverageEnergyAction(info_msg &msg);
        static void     getAverageVisibilityAction(info_msg &msg);
        static void     getAverageFlickerAction(info_msg &msg);

        //SERIAL
        static void     serialGetReferenceAction(info_msg &msg);
        static void     serialGetDutyCycleAction(info_msg &msg);
        static void     serialGetIluminanceAction(info_msg &msg);
        static void     serialGetOccupancyAction(info_msg &msg);
        static void     serialGetAntiWindupAction(info_msg &msg);
        static void     serialGetFeedbackAction(info_msg &msg);
        static void     serialGetExternalIluminanceAction(info_msg &msg);
        static void     serialGetElapsedTimeAction(info_msg &msg);
        static void     serialGetInstantaneousPowerAction(info_msg &msg);
        static void     serialGetAverageEnergyAction(info_msg &msg);
        static void     serialGetAverageVisibilityAction(info_msg &msg);
        static void     serialGetAverageFlickerAction(info_msg &msg);
        static void     serialStreamLuxAction(info_msg &msg);
        static void     serialStreamDutyCycleAction(info_msg &msg);
        //static void     serialGetLastMinuteBufferLuxAction(info_msg &msg);
        //static void     serialGetLastMinuteBufferDutyCycleAction(info_msg &msg);

        //HUB
        static void     foundHubAction(info_msg &msg);

        //WAKE UP
        static void     WakeUpAction(info_msg &msg);
        static void     ackInternalAction(info_msg &msg);
        static void     measureNOlightAction(info_msg &msg);
        static void     measurelightAction(info_msg &msg);
        static void     NotifyThisLightAction(info_msg &msg);
        static void     EndGainsAction(info_msg &msg);

        //TABLE 3
        static void     setLowerBoundOccupiedAction(info_msg &msg);
        static void     setLowerBoundUnoccupiedAction(info_msg &msg);
        static void     getLowerBoundOccupiedAction(info_msg &msg);
        static void     getLowerBoundUnoccupiedAction(info_msg &msg);
        static void     getCurrentLowerBoundAction(info_msg &msg);
        static void     getCurrentEnergyCostAction(info_msg &msg);
        static void     setCurrentEnergyCostAction(info_msg &msg);
        static void     restartAction(info_msg &msg);
        static void     serialGetLowerBoundOccupiedAction(info_msg &msg);
        static void     serialGetLowerBoundUnoccupiedAction(info_msg &msg);
        static void     serialGetCurrentLowerBoundAction(info_msg &msg);
        static void     serialGetCurrentEnergyCostAction(info_msg &msg);
        //Consensus
        static void BeginConsensusAction(info_msg &msg);
        static void ACKConsensusAction(info_msg &msg);
        static void ReceiveConsensusAction_vector0(info_msg &msg);
        static void ReceiveConsensusAction_vector1(info_msg &msg);
        static void ReceiveConsensusAction_vector2(info_msg &msg);
        static void ReceiveConsensusAction_vector3(info_msg &msg);
        static void ReceiveConsensusAction(int i,int node, float value);
        static void ACKBeginConsensusAction(info_msg &msg);
        static void ChangeIterAction(info_msg &msg);
};