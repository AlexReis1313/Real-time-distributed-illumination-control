// Done by Duarte 13/03/2024

#ifndef VARS_H
#define VARS_H

#include <Arduino.h>
#include <ostream>
#include <math.h>
#include <stdio.h>
#include <algorithm>
#include <vector>

#include "pid.h"
#include "parser.hpp"
#include "metrics.hpp"

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
    Metrics          my_metrics;

    float            k;             
    float            b_controller;  
    float            tau;           
    float            gain;          
    float            H_xref;        
    float            H_x;           
    float            u;
    float            Tt;

    float            o_lux;

    float            x_ref;
    float            ref_volts;
    float            vss;
    float            vss_lux;
    bool             inicial;
    bool             occupancy;

    unsigned long    control_interval;
    unsigned long    last_control_time;
    unsigned long    current_time;
};

void                    vars_setup(void);
t_data                  *my(void);

#endif