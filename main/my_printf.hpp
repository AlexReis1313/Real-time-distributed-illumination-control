#ifndef MY_PRINTF_H
#define MY_PRINTF_H

#include <HardwareSerial.h>
#include <cstdarg>

void my_printf(HardwareSerial& serial, const char* fmt, ...);

#endif

