/* 
 * File:   eusart.h
 * Author: Bahtiyar Bayram
 * Comments:
 * Revision history: v1.0
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef EUSART_H
#define	EUSART_H

void EUSART_Init(void);
void EUSART_WriteByte(uint8_t data);
void EUSART_WriteString(uint8_t *str);
uint8_t EUSART_ReadByte(void);

#endif	/* EUSART_H */

