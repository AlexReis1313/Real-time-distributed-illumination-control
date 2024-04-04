#include "includes/CanManager.hpp"
#include "includes/my_aux.h"
#include "includes/distrControl.hpp"
#include "includes/vars.h"

//TABLE 3

//GETTERS
void CanManager::getLowerBoundOccupiedAction(info_msg &msg) {
    //Serial.println("ACTION::Get Lower Bound Occupied Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Get Lower Bound Occupied Action received");
        msg.type = my_type::SERIAL_GET_LOWER_BOUND_OCCUPIED;
        unsigned char data[sizeof(float)];
        float lower_bound_occupied = distrControl::get_lower_bound_occupied();
        memcpy(data, &lower_bound_occupied, sizeof(lower_bound_occupied));
        CanManager::enqueue_message(PICO_ID, msg.type, data, sizeof(data));
    }
}

void CanManager::getLowerBoundUnoccupiedAction(info_msg &msg) {
    //Serial.println("ACTION::Get Lower Bound Unoccupied Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Get Lower Bound Unoccupied Action received");
        msg.type = my_type::SERIAL_GET_LOWER_BOUND_UNOCCUPIED;
        unsigned char data[sizeof(float)];
        float lower_bound_unoccupied = distrControl::get_lower_bound_unoccupied();
        memcpy(data, &lower_bound_unoccupied, sizeof(lower_bound_unoccupied));
        CanManager::enqueue_message(PICO_ID, msg.type, data, sizeof(data));
    }
}

void CanManager::getCurrentLowerBoundAction(info_msg &msg) {
    //Serial.println("ACTION::Get Current Lower Bound Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Get Current Lower Bound Action received");
        msg.type = my_type::SERIAL_GET_CURRENT_LOWER_BOUND;
        unsigned char data[sizeof(float)];
        float current_lower_bound = distrControl::get_lower_bound();
        memcpy(data, &current_lower_bound, sizeof(current_lower_bound));
        CanManager::enqueue_message(PICO_ID, msg.type, data, sizeof(data));
    }
}

void CanManager::getCurrentEnergyCostAction(info_msg &msg) {
    //Serial.println("ACTION::Get Current Energy Cost Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Get Current Energy Cost Action received");
        msg.type = my_type::SERIAL_GET_CURRENT_ENERGY_COST;
        unsigned char data[sizeof(float)];
        float current_energy_cost = distrControl::get_cost();
        memcpy(data, &current_energy_cost, sizeof(current_energy_cost));
        CanManager::enqueue_message(PICO_ID, msg.type, data, sizeof(data));
    }
}

//SETTERS
void CanManager::setLowerBoundOccupiedAction(info_msg &msg) {
    //Serial.println("ACTION::Set Lower Bound Occupied Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Lower Bound Occupied Action received");
        distrControl::set_lower_bound_occupied(value);
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
    }
}

void CanManager::setLowerBoundUnoccupiedAction(info_msg &msg) {
    //Serial.println("ACTION::Set Lower Bound Unoccupied Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Lower Bound Unoccupied Action received");
        distrControl::set_lower_bound_unoccupied(value);
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
    }
}

void CanManager::setCurrentEnergyCostAction(info_msg &msg) {
    //Serial.println("ACTION::Set Current Energy Cost Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Set Current Energy Cost Action received");
        distrControl::set_cost(value);
        CanManager::enqueue_message(PICO_ID, my_type::ACK, nullptr, 0);
    }
}

void CanManager::serialGetLowerBoundOccupiedAction(info_msg &msg) {
    Serial.println("ACTION::Serial Get Lower Bound Occupied Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("O %d %lf\n", msg.sender, value);
    }
}

void CanManager::serialGetLowerBoundUnoccupiedAction(info_msg &msg) {
    Serial.println("ACTION::Serial Get Lower Bound Unoccupied Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("U %d %lf\n", msg.sender, value);
    }
}

void CanManager::serialGetCurrentLowerBoundAction(info_msg &msg) {
    Serial.println("ACTION::Serial Get Current Lower Bound Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("L %d %lf\n", msg.sender, value);
    }
}

void CanManager::serialGetCurrentEnergyCostAction(info_msg &msg) {
    Serial.println("ACTION::Serial Get Current Energy Cost Action received");
    float value;
    memcpy(&value, msg.data, sizeof(float));
    if (PICO_ID == HUB) {
        Serial.printf("c %d %lf\n", msg.sender, value);
    }
}

void CanManager::restartAction(info_msg &msg) {
    Serial.println("ACTION::Restart Action received");
    if (msg.can_id == PICO_ID) {
        Serial.println("ACTION::Restart Action received");
        //distrControl::restart();
    }
}