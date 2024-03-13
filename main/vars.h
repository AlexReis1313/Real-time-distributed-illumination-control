/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vars.h                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: duarte33 <duarte33@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/13 00:38:34 by duarte33          #+#    #+#             */
/*   Updated: 2024/03/13 01:19:12 by duarte33         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Done by Duarte 13/03/2024

#ifndef VARS_H
#define VARS_H

#include <Arduino.h>
#include <ostream>
#include <math.h>
#include <stdio.h>
#include <hardware/flash.h> //for flash_get_unique_id
#include "mcp2515.h"
#include "parser.hpp"
#include "pid.h"

extern const int       LED_PIN;
extern const int       DAC_RANGE;
extern const float     b;
extern const float     m;
extern const int       LDR_port;
extern float           x_ref;

extern pid             my_pid;
extern Parser          my_parser;

extern float            vss;
extern float            y;
extern float            ref_volts;
extern float            vss_lux;
extern float            G ; //0.0041873786
extern float            H_xref; //0.0751975700 para ref=10
extern float            H_x; //0.0592132099 para ref=10
extern bool             gain_setup;


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

float   Volt2LUX(float v_in);
float   LUX2Volt(float LUX_in);
void    calculate_tau(float voltage[], int my_time[], int i);
double  get_H_xref(float x_ref); //x_ref is in lux
double  get_H_x(float x_ref, float vss);
double  get_gain();

#endif