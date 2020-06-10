/*
 * File:   main.c
 * Author: Bahtiyar Bayram
 *
 * Created on March 8, 2020, 8:50 AM
 */


#include "config.h"

#define IN_MODE -2
#define NO_MODE -1
#define W_MODE  1
#define L_MODE  2
#define H_MODE  3
#define T_MODE  4
#define S_MODE  5

#define CONTINUOUS  0
#define SINGLE      1

#define STOP        0
#define RUN         1
#define WAIT        2
#define CALCULATE   3

int8_t mode = NO_MODE;
float mV;
uint8_t pulseWidth, pulseLow, pulseHigh, workingTime;
uint16_t timeCounter = 0;
uint8_t data = 0, state = STOP, workingMode = CONTINUOUS;

// Interrupt Service Routine
void __interrupt() ISR(void) {
    // Timer0 Interrupt - Freq = 100.16 Hz - Period = 0.009984 seconds
    if (INTCONbits.TMR0IF) {
        timeCounter++;
        if(timeCounter >= 100*workingTime) {
            state = STOP;
            timeCounter = 0;
            // Disable Timer 0 Interrupt
            INTCONbits.TMR0IE = 0;
        }
        INTCONbits.TMR0IF = 0;    // Clear the flag
        TMR0 = 100;           // Reset the timer preset count
    }
    
    // If Receive UART interrupt appears
    if(PIR1bits.RCIF) {
        data = EUSART_ReadByte();
        // If data is $ then set to IN_MODE
        if(data == '$' && mode == NO_MODE) {
            mode = IN_MODE;
        }
        // If data is in mode
        else if (mode == IN_MODE) {
            // Data is in Signal Width Mode (current)
            if(data == 'W') {
                mode = W_MODE;
            } 
            // Data is in Signal Low Time Mode
            else if (data == 'L') {
                mode = L_MODE;
            } 
            // Data is in Signal High Time Mode
            else if (data == 'H') {
                mode = H_MODE;
            } 
            // Signal is continuous
            else if (data == 'C') {
                workingMode = CONTINUOUS;
                state = CALCULATE;
                mode = NO_MODE;
            } 
            // Signal is single
            else if (data == 'T') {
                mode = T_MODE;
                workingMode = SINGLE;
            } 
            // Stop command appears
            else if (data == 'S') {
                // Run stop command...
                state = STOP;
                mode = NO_MODE;
            }
        } 
        // Get Signal Width data
        else if (mode == W_MODE) {
            pulseWidth = data;
            mode = NO_MODE;
        } 
        // Get Signal Low Time data
        else if (mode == L_MODE) {
            pulseLow = data;
            mode = NO_MODE;
        } 
        // Get Signal High Time data
        else if (mode == H_MODE) {
            pulseHigh = data;
            mode = NO_MODE;
        } 
        // Get Working Time data
        else if (mode == T_MODE) {
            workingTime = data;
            state = CALCULATE;
            mode = NO_MODE;
        }
    }   
}

// Delay us function
void delay_us(uint16_t time) {
    for(uint16_t i = 0; i < time; ++i) {
        __delay_us(1);
    }
}

void main(void) {
    // Oscillator Initialize
    OSCCONbits.IRCF = 0b1111;   // 16 MHz
    OSCCONbits.SCS = 0b10;      // Internal oscillator
    // Wait until HFINTOSC is ready
    while(!OSCSTATbits.HFIOFR);
    
    // Disable analog pins
    ANSELA = 0;
    ANSELB = 0;
    ANSELC = 0;
    
    // Initialize DAC
    DAC1_Init();
    // Set output voltage in mV
    DAC1_Out(0);
    // Initialize UART
    EUSART_Init();
    
    // Timer0 Initialize
    //Timer0 Period = 0.009984 seconds
    // TMR0 Clock Source Select to Internal Clock (CLKO)
    OPTION_REGbits.T0CS = 0;
    // TMR0 Source Edge Select bit 0 = low/high
    OPTION_REGbits.T0SE = 0;
    // Prescaler is assigned to the Timer0
    OPTION_REGbits.PSA = 0;
    // Set Prescaler to 1:256
    OPTION_REGbits.PS2 = 1;
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 1;
    // Set timer register
    TMR0 = 100;
    
    // Enable UART Receive Interrupt
    PIE1bits.RCIE = 1;
    // Disable Timer 0 Interrupt
    INTCONbits.TMR0IE = 0;
    // Clear Timer 0 interrupt flag
    INTCONbits.TMR0IF = 0;
    // Enable Peripheral Interrupts
    INTCONbits.PEIE = 1;
    // Enable Global Interrupt
    INTCONbits.GIE = 1;
    
    // Set IOs
    TRISBbits.TRISB0 = 0;
    TRISBbits.TRISB1 = 0;
    PORTBbits.RB0 = 0;  // Run LED
    PORTBbits.RB1 = 0;  // Stop LED
    
    // 200ms delay
    __delay_ms(200);        
    
    // Infinite loop
    while(1) {
        // If stop command appears then set output voltage to 0
        if (state == STOP) {
            DAC1_Out(0);
            PORTBbits.RB0 = 0;  // Run LED
            PORTBbits.RB1 = 1;  // Stop LED
            timeCounter = 0;
            // Disable Timer 0 Interrupt
            INTCONbits.TMR0IE = 0;
            INTCONbits.TMR0IF = 0;    // Clear the flag
            TMR0 = 100;           // Reset the timer preset count
            state = WAIT;
        }
        // If new data appears then calculate output voltage
        else if(state == CALCULATE) {
            // Calculate output voltage in mV
            mV = 56.2*pulseWidth - 25.9;
            state = RUN;
        }
        // If output voltage calculated then run commands
        else if (state == RUN) {
            PORTBbits.RB0 = 1;  // Run LED
            PORTBbits.RB1 = 0;  // Stop LED
            
            // If workingMode is continuous then run commands infinitely
            if(workingMode == CONTINUOUS) {
                while(state == RUN) {
                    if(pulseLow == 0) {
                        DAC1_Out(mV);
                        delay_us(pulseHigh);
                    }
                    else {
                        DAC1_Out(mV);
                        delay_us(pulseHigh);
                        DAC1_Out(0);
                        delay_us(pulseLow);
                    }
                }
            }
            // If workingMode is single then run commands for a given time
            else if(workingMode == SINGLE) {
                // Enable Timer 0 Interrupt
                INTCONbits.TMR0IE = 1;
                // Run commands for a given time
                while(state == RUN) {
                    if(pulseLow == 0) {
                        DAC1_Out(mV);
                        delay_us(pulseHigh);
                    }
                    else {
                        DAC1_Out(mV);
                        delay_us(pulseHigh);
                        DAC1_Out(0);
                        delay_us(pulseLow);
                    }
                }
                state = STOP;
            }
        }
    }
}
