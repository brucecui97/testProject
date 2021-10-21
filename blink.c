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


volatile int currentDivisor = 8;
volatile int pwmPeriod = 10000;
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    LEDInit();
    CSCTL0_H = 0xA5;
    CSCTL1 |= DCOFSEL0 + DCOFSEL1;             // Set max. DCO setting =8MHz
    CSCTL2 = SELA_3 + SELS_3 + SELM_3;        // set ACLK = SMCLK = DCO/8
    CSCTL3 = DIVA_3 + DIVS_3 + DIVM_3;        // set all dividers

    P3DIR |= BIT4 + BIT5;                       // P1.6 and P1.7 output
    P3SEL0 |= BIT4 + BIT5;                      // P1.6 and P1.7 options select




    TB1CCR0 = pwmPeriod;                         // PWM Period

    TB1CCTL1 = OUTMOD_7;                      // CCR1 reset/set
    TB1CCR1 = pwmPeriod/8;                            // CCR1 PWM duty cycle

//    TB1CCTL2 = OUTMOD_7;                      // CCR1 reset/set
//    TB1CCR2 = pwmPeriod*1/4;                            // CCR1 PWM duty cycle

    TB1CTL = TBSSEL_2 + MC_1 + TBCLR;         // SMCLK, up mode, clear TAR

    //1. Configure P4.0 as a digital input.
    P4DIR &= ~BIT0;
    P4DIR &= ~BIT1;
    //2. The switch S1 is connected to P4.0 on the EXP Board. Enable the internal pull-up resistors for the switch.
    P4REN |= BIT0 + BIT1;
    P4OUT |= BIT0+BIT1;

    //3. Set P4.0 to get interrupted from a rising edge (i.e. an interrupt occurs when the user lets go of the button).
    //Enable local and global interrupts.
    //P4IES &=~BIT0;
    //P4IES &=~BIT1;
    P4IES |=BIT0 + BIT1;

    P4IE |= BIT0 + BIT1; //enable p4.1 IRQ
    __enable_interrupt();
    P4IFG &= ~BIT0;//Clear P4.0 IRQ Flag
    P4IFG &= ~BIT1;//Clear P4.0 IRQ Flag

    __bis_SR_register(LPM0_bits);             // Enter LPM0
    __no_operation();                         // For debugger
  }

#pragma vector = PORT4_VECTOR
__interrupt void ISR_Port4_S0(void){
    //LEDToggle(8);

    if ((P4IFG&BIT0)==BIT0){
        //go high
        if(currentDivisor>1){
            currentDivisor = currentDivisor -1;
        }
        TB1CCR1 = pwmPeriod/currentDivisor;
        P4IFG &= ~BIT0;//Clear P4.0 IRQ Flag
    }
    else{
        if(currentDivisor<8){
            currentDivisor = currentDivisor+1;
            TB1CCR1 = pwmPeriod/currentDivisor;
                }
        //go low
        else if (currentDivisor==8){
            TB1CCR1 = 0;
        }
        P4IFG &= ~BIT1;//Clear P4.0 IRQ Flag
    }
}



