// Done by Duarte 13/03/2024
#include <map> 

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
    unsigned long    initial_time;
    unsigned long    initial_time_local;
    unsigned long    elapsed_time;
    unsigned long    last_time_stream;

    float            nominal_power;
    
    bool             stream_lux;
    bool             stream_duty_cycle;
    int              stream_id;

    std::vector<int> list_Nr_detected_IDS; // List to store the number of detected IDs
    std::vector<unsigned char> list_IDS; // List to store the IDs of other picos
    std::vector<unsigned char> list_nodes; // List to store the IDs of other picos
    std::vector<unsigned char> list_Ack;
    int nr_ckechIn_Nodes;
    int THIS_NODE_NR;
    std::map<unsigned char, int> id_to_node;
    std::map<int, unsigned char> node_to_id;

    //consensus
    int consensus_iteration;
    int consensus_maxIterations;
    bool consensus_ongoing;
    bool send_consensus;
    std::vector<int> list_Nr_detected_consensus; //contins how many consensus the others have receive. When all have received nr_ckechIn_Nodes - 1, then move to next iter
    std::vector<int> list_consesus_received_vector;
    bool sendingConsensus_begin;


};

void                    vars_setup(void);
t_data                  *my(void);

#endif