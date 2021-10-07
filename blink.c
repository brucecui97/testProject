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


#define LED1 BIT0
#define LED2 BIT1
#define LED3 BIT2
#define LED4 BIT3
#define LED5 BIT4
#define LED6 BIT5
#define LED7 BIT6
#define LED8 BIT7


//array of led pins
unsigned char leds[]={LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8};

void LEDInit(void)
{
    //first four bits
    PJDIR |= LED1 + LED2 + LED3 + LED4;
    //Leds Off
    PJOUT &= ~(LED1 + LED2 + LED3 + LED4);
    //last four leds
    P3DIR |= LED5 + LED6 + LED7 + LED8;
    //Leds Off
    P3OUT &= ~(LED5 + LED6 + LED7 + LED8);
}

void LEDOn(unsigned char LEDn)
{
    if ((LEDn > 0) && (LEDn <= 4))
    {
        PJOUT |= leds[--LEDn];
    }
    if ((LEDn > 4) && (LEDn <= 8))
    {
        P3OUT |= leds[--LEDn];
    }
}
void LEDOff(unsigned char LEDn)
{
    if ((LEDn > 0) && (LEDn <= 4))
    {
        PJOUT &= ~leds[--LEDn];
    }
    if ((LEDn > 4) && (LEDn <= 8))
    {
        P3OUT &= ~leds[--LEDn];
    }
}
void LEDToggle(unsigned char LEDn)
{
    if ((LEDn > 0) && (LEDn <= 4))
    {
        PJOUT ^= leds[--LEDn];
    }
    if ((LEDn > 4) && (LEDn <= 8))
    {
        P3OUT ^= leds[--LEDn];
    }
}

void main(void)
{
  LEDInit();
  // Stop WDT
  WDTCTL = WDTPW + WDTHOLD;

  //1. Configure P4.0 as a digital input.
  P4DIR &= ~BIT0;

  //2. The switch S1 is connected to P4.0 on the EXP Board. Enable the internal pull-up resistors for the switch.
  P4REN |= BIT0;
  P4OUT |= BIT0;

  //3. Set P4.0 to get interrupted from a rising edge (i.e. an interrupt occurs when the user lets go of the button).
  //Enable local and global interrupts.
  P4IES &=~BIT0;

  P4IE |= BIT0; //enable p4.1 IRQ
  __enable_interrupt();
  P4IFG &= ~BIT0;//Clear P4.0 IRQ Flag


  //4. Configure P3.7 as an output (this is connected to LED8).
  //5. Write an interrupt service routine to toggle LED8 when S1 provides a rising edge.
  while(1){}
}

//--- ISRs--------------------//
#pragma vector = PORT4_VECTOR
__interrupt void ISR_Port4_S0(void){
    LEDToggle(8);
    P4IFG &= ~BIT0;//Clear P4.0 IRQ Flag
}


