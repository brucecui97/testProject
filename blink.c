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

  P1DIR |= BIT0;
  P1OUT |= BIT0;

  TB0CCTL0 = CCIE;                          // TACCR0 interrupt enabled
  TB0CCR0 = 50000;
  TB0CTL = TBSSEL_2 + MC_2;                 // SMCLK, continuous mode

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0 w/ interrupt
}

// Timer B0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) Timer_B (void)
#else
#error Compiler not supported!
#endif
{
    LEDToggle(2);
  P1OUT ^= BIT0;
  TB0CCR0 += 50000;                         // Add Offset to TACCR0
}
