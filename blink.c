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

// the setup function runs once when you press reset or power the board
void setup() {

    q_init(&q, sizeof(Rec), 10, IMPLEMENTATION, false);
}

// the loop function runs over and over again forever
int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    setup();
    unsigned int i;

    for (i = 0 ; i < sizeof(tab)/sizeof(Rec) ; i++)
    {
        Rec rec = tab[i];
        q_push(&q, &rec);
    }

    for (i = 0 ; i < sizeof(tab)/sizeof(Rec) ; i++)
    {
        q_pop(&q, &myRec);
    }

    while(1);
}
