/*
 * File:   dac.c
 * Author: Bahtiyar Bayram
 *
 * Created on June 10, 2020, 10:22 PM
 */


#include "config.h"

// DAC1 Initialize function
void DAC1_Init(void) {
    DAC1CON0bits.DAC1PSS = 0b00;
    DAC1CON0bits.DAC1NSS = 0;
    DAC1CON0bits.DAC1OE1 = 1;
    DAC1CON0bits.DAC1OE2 = 0;
    DAC1CON0bits.DAC1EN = 1;
}

// DAC Out function. Voltage in mV
void DAC1_Out(uint32_t voltage) {
    voltage = ((voltage*51)/1000)+1;
    if(voltage < 51)
        DAC1CON1 = voltage;
    else if(voltage < 101)
        DAC1CON1 = voltage+1;
    else if(voltage < 151)
        DAC1CON1 = voltage+2;
    else if(voltage < 201)
        DAC1CON1 = voltage+3;
    else if(voltage < 251)
        DAC1CON1 = voltage+4;
    else
        DAC1CON1 = 255;
}