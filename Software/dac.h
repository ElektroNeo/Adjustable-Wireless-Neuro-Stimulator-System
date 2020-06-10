/* 
 * File:   dac.h
 * Author: Bahtiyar Bayram
 * Comments:
 * Revision history: v1.0
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef DAC_H
#define	DAC_H

void DAC1_Init(void);
void DAC1_Out(uint32_t voltage);

#endif	/* DAC_H */

