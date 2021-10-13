//***************************************************************************************
//  MSP430 Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 by xor'ing P1.0 inside of a software loop.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP430x5xx
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  Texas Instruments, Inc
//  July 2013
//***************************************************************************************

#include "msp430fr5739.h"

void main(void)
{
    //1. Configure P2.7 to output high to power the accelerometer.
    P2DIR |= BIT7;
    P2OUT |= BIT7;

    //2. Set up the ADC to sample from ports A12, A13, and A14.

  while(1){
  }
}
