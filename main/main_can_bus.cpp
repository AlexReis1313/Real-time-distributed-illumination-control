// Done by Duarte 30/03/2024
#include "includes/vars.h"
#include "includes/aux.h"
#include "includes/vars_can_bus.h"
#include "includes/CanManager.hpp"

CanManager canBus();

void setup() {
    //Setup serial communication
    Serial.begin(115200);
    analogReadResolution(12);
    analogWriteFreq(30000); //30KHz
    analogWriteRange(4096); //Max PWM

    //Setup controller, metrics and parser
    vars_setup();
}

void setup1() {
    canBus.flashIDsetup();
    canBus.begin(CAN_1000KBPS);
    canBus.setUpFiltersAndMasks();
}

void loop() {
    actions_rotine();
    controller_rotine();
}

void loop1() {
    /* Two buffers (RXB0 and RXB1) */
    can_frame frame, *ptr_frame;
    canBus.sendMessage1to0(&frame, &ptr_frame);
}

info_msg extract_message(can_frame* frame) {
  info_msg result;
  memset(&result, 0, sizeof(result));
  result.size = frame->can_dlc - 2;  
  result.type = (my_type)frame->data[1];
  result.sender = frame->data[0];
  memcpy(result.data, &frame->data[2], result.size);

  //todo
  if (frame->can_dlc > 6)
    result.data[6] = (frame->can_id & 0x0000ff00) >> 8;
  if (frame->can_dlc > 7)
    result.data[7] = (frame->can_id & 0x00ff0000) >> 16;
  return result;
}

void actions_rotine() {
    can_frame *frame;

    while (rp2040.fifo.available()) {
        if (!rp2040.fifo.pop_nb((uint32_t *)&frame))
            break;
        info_msg pm = extract_message(frame);
        msg_t message_type = pm.type;
        uint8_t sender = pm.sender;
        uint8_t *data = pm.data;
        
        std::map<int, eventFunction>::iterator it = _actionMap.find(message_type);
        if (_actionMap.find(message_type) != _actionMap.end()){
            it->second(pm);
            Serial.print("Type: "); Serial.print(it->first);
        }
    }
}

void controller_rotine() {
    my()->current_time = millis();
    if (my()->current_time - my()->last_control_time >= my()->control_interval) {
        
        my()->vss = get_adc_digital_filter(40, 10) * 3.3 / 4095; // Convert ADC (analog to digital converter) to volts
        my()->vss_lux = Volt2LUX(my()->vss); //Get LDR value in lux

        get_H_xref();
        get_H_x();

        my()->my_pid.setBcontroller((1 / (my()->H_xref * my()->gain * my()->k))); //Change b of controller based on the H_ref and H_x    
        my()->u = my()->my_pid.compute_control(my()->ref_volts, my()->vss); // Compute control
        analogWrite(my()->LED_PIN, (my()->u)); //Apply control signal to LED
        my()->my_pid.housekeep(my()->ref_volts, my()->vss);

        print_vars();
        //float percnt_dutycycle = ((my()->vss * 4095) / 3.3) / 4095;
        //my()->my_metrics.updateMetrics(my()->x_ref, my()->vss_lux , percnt_dutycycle);
        my()->last_control_time = my()->current_time;
    }
}

