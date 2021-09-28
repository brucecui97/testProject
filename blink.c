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
  unsigned char i;
  // Stop WDT
  WDTCTL = WDTPW + WDTHOLD;
  LEDInit();
  while(1)
  {

      LEDOn(1);
      LEDOn(4);
      LEDOn(7);
      LEDOn(8);

      LEDToggle(2);
      LEDToggle(3);
      LEDToggle(5);
      LEDToggle(6);

      __delay_cycles(100000);
  }
}
