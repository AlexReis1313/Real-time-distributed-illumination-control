#include <hardware/flash.h>
#include "mcp2515.h"

// Unique identifiers for inter-core communication commands
enum InterCoreCmd {
    ReadData =  1,   // From core 1 to core 0: contains data read (16 bit)
    WriteData = 2,   // From core 0 to core 1: contains data to write (16 bit)
    ErrorData = 3    // From core 1 to core 0: contains regs CANINTF, EFLG
};

data[1]("SET_OCCUPANCY") 

// Can-bus setup
struct canBus_vars {
    uint8_t this_pico_flash_id[8];
    uint8_t node_address;  // Node address (last byte of the flash ID)
    struct can_frame canMsgTx;
    struct can_frame canMsgRx;
    unsigned long counterTx;
    unsigned long counterRx;
    MCP2515::ERROR err;
    unsigned long time_to_write;
    unsigned long write_delay;
    const byte interruptPin;

    can_bus_vars() : counterTx(0), counterRx(0), \
                        write_delay(1000), interruptPin(20) {}
};

class CanBus {
    private:
        MCP2515 canController;
        uint8_t interruptPin;
        volatile bool dataAvailable;
        struct can_frame canMsgTx, canMsgRx;

        // Interrupt Service Routine
        static void ISR_wrapper() {
            instance->canInterrupt(); // Call the member ISR
        }

    public:
        static CanBus *instance; // Static instance pointer for ISR access

        CanBus(uint8_t intPin) : canController(spi0, 17, 19, 16, 18, 10000000), interruptPin(intPin), dataAvailable(false) {
            instance = this; // Set the instance pointer for ISR access
        }

        void begin(long bitrate) {
            // Initialize CAN controller
            canController.reset();
            canController.setBitrate(bitrate);
            canController.setNormalMode();
            
            // Initialize interrupt pin for CAN-BUS messages
            pinMode(interruptPin, INPUT_PULLUP);
            attachInterrupt(digitalPinToInterrupt(interruptPin), ISR_wrapper, FALLING);
        }

        void canInterrupt() {
            dataAvailable = true; // Set flag when CAN data is available
        }

        void sendMessage(const struct can_frame &msg) {
            if (canController.sendMessage(&msg) == MCP2515::ERROR_OK) {
                // Handle post send operations
            }
        }

        bool receiveMessage(struct can_frame &msg) {
            if (dataAvailable && canController.readMessage(&msg) == MCP2515::ERROR_OK) {
                dataAvailable = false; // Reset the flag
                return true;
            }
            return false;
        }

        bool data_available() {
            return dataAvailable;
        }
};

CanBus *CanBus::instance = nullptr; // Initialize static instance pointer
CanBus canBus(interruptPin);
canBus_vars canbus_vars;

void setup() {
    rp2040.idleOtherCore();
    flash_get_unique_id(canbus_vars.this_pico_flash_id);
    rp2040.resumeOtherCore();
    canbus_vars.node_address = this_pico_flash_id[7];

    //Setup serial communication
    Serial.begin(115200);
    analogReadResolution(12);
    analogWriteFreq(30000); //30KHz
    analogWriteRange(4096); //Max PWM

    //Setup controller, metrics and parser
    vars_setup();
    my()->my_pid.setBcontroller((1 / (my()->H_xref * my()->gain * my()->k)));    

    //node.set_node();

    //Setup CAN-BUS
    canBus.begin(CAN_500KBPS);// Initialize CAN-BUS at 500kbps
    canbus_vars.time_to_write = millis() + canbus_vars.write_delay;
}

void setup1() {
    /* 
      If the received message's identifier matches the filter, it is accepted and processed further.
      A mask is a bit pattern that determines which bits in the filter are significant for comparison
     */
    rp2040.idleOtherCore();
    flash_get_unique_id(canbus_vars.this_pico_flash_id);
    rp2040.resumeOtherCore();

    canBus.begin(CAN_500KBPS);
    canbus_vars.node_address = this_pico_flash_id[7];

    canBus.setBitrate(CAN_1000KBPS);

    //Sets up Filter 0 to accept all messages where the last byte of the identifier matches 0x000000ff
    canBus.setFilterMask(MCP2515::MASK0, 0, 0x000000ff); //last byte is the significant one
    canBus.setFilter(MCP2515::RXF0, 0, 0); //accept all messages

    //Sets up Filter 1 to accept all messages where the last byte of the identifier matches the node address
    canBus.setFilterMask(MCP2515::MASK1, 0, 0x000000ff);
    canBus.setFilter(MCP2515::RXF1, 0, canbus_vars.node_address);

    //canBus.setFilter(MCP2515::RXF2, 0);
    //canBus.setFilter(MCP2515::RXF3, canbus_vars.node_address);

    canBus.setNormalMode();
}

void loop() {
    can_frame frm;
    uint32_t msg;
    uint8_t b[4];

    if (canBus.dataAvailable()) {
        canBus.readData();
    }
    if (millis() >= canbus_vars.time_to_write ) {
        b[3] = WriteData;
        b[2] = canbus_vars.node_address;
        b[0] = canbus_vars.counterTx;
        b[1] = (canbus_vars.counterTx >> 8);
        // Push data into fifo (simulate)
        // rp2040.fifo.push(bytes_to_msg(b));
        Serial.print(">>>>>> Sending ");
        print_message(canbus_vars.counterTx, b[2], b[2], canbus_vars.counterTx);
        canbus_vars.counterTx++;
        canbus_vars.time_to_write = millis() + canbus_vars.write_delay;
    }
    //run controller here
}

//Unpacks an unsigned int into its constituting 4 bytes
void msg_to_bytes(uint32_t msg, uint8_t * bytes) {
    bytes[0] = msg; bytes[1] = (msg >> 8);
    bytes[2] = (msg >> 16); bytes[3] = (msg >> 24);
}
//Packs the CAN frame contents into an unsigned int
uint32_t can_frame_to_msg(can_frame * frm) {
    uint8_t b[4];
    b[3] = ReadData; b[2] = frm->can_id;
    b[1] = frm->data[1]; b[0] = frm->data[0];
    return bytes_to_msg(b);
}
//Packs the CAN error flags into an unsigned int
uint32_t error_flags_to_msg(uint8_t canintf, uint8_t eflg) {
    uint8_t b[4];
    b[3] = ErrorData; b[2] = 0;
    b[1] = canintf; b[0] = eflg;
    return bytes_to_msg(b);
}

void loop1() {
    /* Two buffers (RXB0 and RXB1) */
    can_frame frame; //*ptr_frame;
    uint32_t  msg;
    uint8_t   b[4];

    uint8_t irq = can0.getInterrupts(); // Get the interrupt flags
    if (irq & MCP2515::CANINTF_RX0IF) { // Check if interrupt is from RXB0
        can0.readMessage(MCP2515::RXB0, &frame);
        rp2040.fifo.push_nb(can_frame_to_msg(&frame));
        //process_can_frame(&frame);
    }
    if (irq & MCP2515::CANINTF_RX1IF) { // Check if interrupt is from RXB1
        can0.readMessage(MCP2515::RXB1, &frame);
        rp2040.fifo.push_nb(can_frame_to_msg(&frame));
        //process_can_frame(&frame);
    }
    if(rp2040.fifo.pop_nb(&msg)) { //message from 0 to 1 
        msg_to_bytes( msg , b );
        if(b[3] == WriteData) {
            frame.can_id = b[2];
            frame.can_dlc = 2;
            frame.data[1] = b[1];
            frame.data[0] = b[0];
            can0.sendMessage(&frame);
        }
        uint8_t irq = can0.getInterrupts();
        uint8_t err = can0.getErrorFlags();
        rp2040.fifo.push_nb(error_flags_to_msg(irq, err));
    }
}


