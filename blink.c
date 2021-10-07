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
    //--
    //WDTCTL = WDTPW | WDTHOLD;                 // Stop WDT
    WDTCTL = WDTPW + WDTHOLD;
    CSCTL0_H = 0xA5;
    //1. Set up SMCLK to run on the DCO.
    CSCTL2 = SELS_3;
    //2. Configure the DCO to run at 8 MHz.
    CSCTL1 = DCOFSEL_3;
    //CSCTL1 &= ~BIT7; //want to do + DCORSEL_0 above but can't??
    //3. Set up the SMCLK with a divider of 32
    CSCTL3 = DIVS_5;

    //4. Configure P3.4 as an output and set it to output the SMCLK. The SMCLK frequency should be ~250 kHz.
    //Check it using an oscilloscope
    P3DIR |= BIT4;
    P3SEL1|=BIT4;
    P3SEL0 |=BIT4;
    //P3OUT &= ~BIT4;

  while(1){
  }
}
