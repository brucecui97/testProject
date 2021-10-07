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

int myVariable = 10;
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    CSCTL0_H = 0xA5;
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;             // Set max. DCO setting =8MHz
    CSCTL2 = SELA_3 + SELS_3 + SELM_3;        // set ACLK = SMCLK = DCO/8
    CSCTL3 = DIVA_3 + DIVS_3 + DIVM_3;        // set all dividers

    P3DIR |= BIT4 + BIT5;                       // P1.6 and P1.7 output
    P3SEL0 |= BIT4 + BIT5;                      // P1.6 and P1.7 options select

    volatile  int b = myVariable +2;

    int pwmPeriod = 1992;
    TB1CCR0 = pwmPeriod;                         // PWM Period

    TB1CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TB1CCR1 = pwmPeriod/2;                            // CCR1 PWM duty cycle

    TB1CCTL2 = OUTMOD_7;                      // CCR1 reset/set
    TB1CCR2 = pwmPeriod*1/4;                            // CCR1 PWM duty cycle

    TB1CTL = TBSSEL_2 + MC_1 + TBCLR;         // SMCLK, up mode, clear TAR

    //setup Timer A
    LEDInit();

    P1SEL0 |= BIT0;
    TA0CCTL1 = CCIE + CM_3 + CCIS_0 + CAP;                          // TACCR0 interrupt enabled
    TA0CTL = TASSEL_2 + MC_2;                 // SMCLK, UP mode            // SMCLK, UP mode

    __enable_interrupt();
    __bis_SR_register(LPM0_bits);             // Enter LPM0
    __no_operation();                         // For debugger
  }

#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer_A (void)
{
  TA0CCTL1&= ~CCIFG;
  LEDToggle(2);
  P1OUT ^= BIT0;
}
