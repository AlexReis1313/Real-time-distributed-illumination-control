// Done by Duarte 13/03/2024

#ifndef VARS_H
#define VARS_H

#include <Arduino.h>
#include <ostream>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <hardware/flash.h> //for flash_get_unique_id
#include "mcp2515.h"
#include "parser.hpp"
#include "pid.h"

typedef struct s_data		t_data;
typedef struct s_time_vars  t_time_vars;

struct s_data
{
    int              LED_PIN;
    int              LDR_port;
    int              DAC_RANGE;

    float            b;
    float            m;
    pid              my_pid;
    Parser           my_parser;

    float            x_ref;
    float            ref_volts;
    float            vss;
    float            vss_lux;

    float            k;             //control variable
    float            b_controller;  //control variable
    float            b_factor;      //control variable
    float            tau;           //tau = Ti
    float            gain;          //0.0041873786
    float            H_xref;        //0.0751975700 para ref=10
    float            H_x;           //0.0592132099 para ref=10
    float            u;
    float            Tt;
};

struct s_time_vars
{
    //For get_vss_non_blocking
    int                 n_measurements;
    int                 measurement_index;
    unsigned long       last_measurement_time;
    unsigned long       measurement_interval; // Time between measurements in milliseconds
    bool                measurement_complete;
    float               median;
    int                 mid_idx;
    
    //For void loop
    unsigned long       last_control_time;
    unsigned long       control_interval;
    unsigned long       current_time; 
};

// Can-bus setup
extern uint8_t         this_pico_flash_id[8], node_address;
extern struct can_frame canMsgTx, canMsgRx;
extern unsigned long   counterTx, counterRx;
extern MCP2515::ERROR  err;
extern unsigned long   time_to_write;
extern unsigned long   write_delay;
extern const byte      interruptPin;
extern volatile byte   data_available;
extern MCP2515         can0;

extern float           voltage[400];
extern int             my_time[400];

float       Volt2LUX(float v_in);
float       LUX2Volt(float LUX_in);
void        calculate_tau(float voltage[], int my_time[], int i);
void        get_H_xref(); //x_ref is in lux
void        get_H_x();
void        get_gain();
int         get_vss_non_blocking();

void        vars_setup(void);
t_time_vars *time_vars(void);
t_data      *my(void);
void        time_vars_setup(void);


#endif