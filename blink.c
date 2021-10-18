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
#include <cQueue.h>

#define IMPLEMENTATION  FIFO
#define MAXSIZE  10
#define START_BYTE  255


typedef struct strRec {
    uint8_t    entry1;
} Rec;

Rec tab[6] = {
    {1},
    {2},
    {3},
    {4},
    {5},
    {6}
};

Queue_t     q;  // Queue declaration
volatile Rec myRec;
volatile char commandByte;
volatile char dataByte1;
volatile char dataByte2;
volatile char escapeByte;

enum nextByteType{START, COMMAND, DATA1, DATA2, ESCAPE};

// the setup function runs once when you press reset or power the board
void setup() {

    q_init(&q, sizeof(Rec), MAXSIZE, IMPLEMENTATION, false);
}

// the loop function runs over and over again forever
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    setup();

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

    unsigned int i;

    for (i = 0 ; i < sizeof(tab)/sizeof(Rec) ; i++)
    {
        Rec rec = tab[i];
        q_push(&q, &rec);
    }

    for (i = 0 ; i < sizeof(tab)/sizeof(Rec) ; i++)
    {
        q_pop(&q, &myRec);
        UCA0TXBUF = myRec.entry1;
    }

    while(1){
        Rec tempRec;

        if(q_pop(&q, &tempRec)){
            if(tempRec.entry1==START_BYTE){

            }
        }
        //UCA0TXBUF = q.cnt;
        //UCA0TXBUF = '!';
        //__delay_cycles(2000000);
    }
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{

    unsigned char RxByte;
    RxByte = UCA0RXBUF;

    if (RxByte == 13){
        if (q.cnt == 0){
            UCA0TXBUF = '!';
        }
        else{
            q_pop(&q, &myRec);
            UCA0TXBUF = myRec.entry1;
        }


    }

    else{
        if (q.cnt<MAXSIZE){
        while ((UCA0IFG & UCTXIFG)==0);
        q_push(&q, &RxByte);
        }
        else{
            UCA0TXBUF = '!';
        }
    }
}

