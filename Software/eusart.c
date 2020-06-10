/*
 * File:   eusart.c
 * Author: Bahtiyar Bayram
 *
 * Created on June 10, 2020, 10:23 PM
 */


#include "config.h"

//USART Initialize function
void EUSART_Init(void) {
    // Unlock PPS
    INTCONbits.GIE = 0;
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCK = 0x00;
    
    // RX input for RC7 pin
    RXPPSbits.RXPPS = 0b10111;
    //RC7PPSbits.RC7PPS = 0b10101;
    // TX output for RC6 pin
    //CKPPSbits.CKPPS = 0b10110;
    RC6PPSbits.RC6PPS = 0b10100;
    
    // Lock PPS
    PPSLOCK = 0x55;
    PPSLOCK = 0xAA;
    PPSLOCK = 0x01;
    
    // 16-bit Baud Rate Generator is used
    BAUD1CONbits.BRG16 = 1;
    // Enable Serial Port
    RC1STAbits.SPEN = 1;
    // Enable receiver
    RC1STAbits.CREN = 1;
    // Asynchronous mode
    TX1STAbits.SYNC = 0;
    // Transmit enabled
    TX1STAbits.TXEN = 1;
    // High Baud Rate Speed
    TX1STAbits.BRGH = 1;

    // 9600 baud rate @16MHz
    SP1BRGL = 0xA0;
    SP1BRGH = 0x01;
}

void EUSART_WriteByte(uint8_t data) {
    // Enable transmission
    TXSTAbits.TXEN = 1;
	while(!PIR1bits.TXIF){}
    // Send data
    TX1REG = data;
    while(!TXSTAbits.TRMT){}
    // Disable transmission
    TXSTAbits.TXEN = 0;
    
}

void EUSART_WriteString(uint8_t *str) {
    for(uint8_t i = 0; str[i] != '\0'; i++) {
        EUSART_WriteByte(str[i]);
    }
}

uint8_t EUSART_ReadByte(void) {
    while(!PIR1bits.RCIF);
    
    if(RC1STAbits.OERR){
        // EUSART error - restart
        RC1STAbits.CREN = 0; 
        RC1STAbits.CREN = 1; 
    }

    return RC1REG;
}
