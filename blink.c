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
#include "cQueue.h"
#define IMPLEMENTATION  FIFO

typedef struct myByte {
    unsigned int myInt;
} myByte;

Queue_t     q;  // Queue declaration


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

/**
 * main.c
 */
int main(void)
{
    //struct point *p = &my_point;  /* p is a pointer to my_point */

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer



    //myQueue = newQueue(50);

    LEDInit();
    // Configure clocks
    CSCTL0 = 0xA500;                        // Write password to modify CS registers
    CSCTL1 = DCOFSEL0 + DCOFSEL1;           // DCO = 8 MHz
    CSCTL2 = SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // MCLK = DCO, ACLK = DCO, SMCLK = DCO

    // Configure ports for UCA0
    P2SEL0 &= ~(BIT0 + BIT1);
    P2SEL1 |= BIT0 + BIT1;

    // Configure UCA0
    UCA0CTLW0 = UCSSEL0;
    UCA0BRW = 52;
    UCA0MCTLW = 0x4900 + UCOS16 + UCBRF0;
    UCA0IE |= UCRXIE;

    // global interrupt enable
    _EINT();

    while (1)
    {
        UCA0TXBUF = '\n';
        __delay_cycles(100000);
    }

    return 0;
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    unsigned char RxByte;
    RxByte = UCA0RXBUF;
    while ((UCA0IFG & UCTXIFG)==0);
    UCA0TXBUF = RxByte;
    if (RxByte == 'j'){
        LEDOn(1);
    }
    else if (RxByte == 'k'){
        LEDOff(1);
    }
    while ((UCA0IFG & UCTXIFG)==0);
    UCA0TXBUF = RxByte;
    while ((UCA0IFG & UCTXIFG)==0);
    UCA0TXBUF = RxByte+1;
}

