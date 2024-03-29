#include <hardware/flash.h>
#include "mcp2515.h"

// Unique identifiers for inter-core communication commands
enum InterCoreCmd {
    ReadData = 1,  // From core 1 to core 0: contains data read (16 bit)
    WriteData,     // From core 0 to core 1: contains data to write (16 bit)
    ErrorData      // From core 1 to core 0: contains regs CANINTF, EFLG
};

// Can-bus setup
uint8_t         this_pico_flash_id[8], node_address;//node address(last byte of the flash ID)
struct can_frame canMsgTx, canMsgRx;
unsigned long   counterTx {0}, counterRx {0};
MCP2515::ERROR  err;
unsigned long   time_to_write;
unsigned long   write_delay {1000};
const byte      interruptPin {20};
//volatile byte   data_available {false};
//MCP2515         can0 {spi0, 17, 19, 16, 18, 10000000};

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

void setup() {
    rp2040.idleOtherCore();
    flash_get_unique_id(this_pico_flash_id);
    rp2040.resumeOtherCore();

    node_address = this_pico_flash_id[7];

    Serial.begin(115200);
    analogReadResolution(12);
    analogWriteFreq(30000); //30KHz
    analogWriteRange(4096); //Max PWM

    //controller.set_controller();
    //node.set_node();

    canBus.begin(CAN_500KBPS);// Initialize CAN-BUS at 500kbps
    time_to_write = millis() + write_delay;
}

void setup1() {
    /* 
      If the received message's identifier matches the filter, it is accepted and processed further.
      A mask is a bit pattern that determines which bits in the filter are significant for comparison
     */
    rp2040.idleOtherCore();
    flash_get_unique_id(this_pico_flash_id);
    rp2040.resumeOtherCore();

    canBus.begin(CAN_500KBPS);
    node_address = this_pico_flash_id[7];

    canBus.setBitrate(CAN_1000KBPS);

    //Sets up Filter 0 to accept all messages where the last byte of the identifier matches 0x000000ff
    canBus.setFilterMask(MCP2515::MASK0, 0, 0x000000ff); //last byte is the significant one
    canBus.setFilter(MCP2515::RXF0, 0, 0); //accept all messages

    //Sets up Filter 1 to accept all messages where the last byte of the identifier matches the node address
    canBus.setFilterMask(MCP2515::MASK1, 0, 0x000000ff);
    canBus.setFilter(MCP2515::RXF1, 0, node_address);

    canBus.setFilter(MCP2515::RXF2, 0);
    canBus.setFilter(MCP2515::RXF3, node_address);

    canBus.setNormalMode();
}

void loop() {
    can_frame frm;
    uint32_t msg;
    uint8_t b[4];

    if (canBus.dataAvailable()) {
        canBus.readData();
    }
    if (millis() >= time_to_write ) {
        b[3] = WriteData;
        b[2] = node_address;
        b[0] = counterTx;
        b[1] = (counterTx >> 8);
        // Push data into fifo (simulate)
        // rp2040.fifo.push(bytes_to_msg(b));
        Serial.print(">>>>>> Sending ");
        print_message(counterTx, b[2], b[2], counterTx);
        counterTx++;
        time_to_write = millis() + write_delay;
    }
    //run controller here
}

// Core 1 loop function
void loop1(CanBus &can) {
    can_frame frm, *received_frm;

    uint8_t irq = can0.getInterrupts();
    if (irq & MCP2515::CANINTF_RX0IF)
    {
        can0.readMessage(MCP2515::RXB0, &frm);
        process_can_frame(&frm);
    }
    if (irq & MCP2515::CANINTF_RX1IF)
    {
        can0.readMessage(MCP2515::RXB1, &frm);
        process_can_frame(&frm);
    }
    if (rp2040.fifo.pop_nb((uint32_t *)&received_frm))
    {
        can0.sendMessage(received_frm);
        delete received_frm;
    }
}

void print_message(int number, int node, int id, int val) {
    Serial.print(number);
    Serial.print(" at node ");
    Serial.print(node,