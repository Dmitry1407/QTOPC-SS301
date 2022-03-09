#include <windows.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

using namespace std;

//Functions
uint16_t CalcCRC(uint8_t*, uint8_t);

//Variables
uint8_t ADDR, KOP;
uint16_t CRC;
short LenOut;
short Pila=0, kPila=1;

union u_float
{
    float   flt;
    uint8_t    data[sizeof(float)];
} Param;
