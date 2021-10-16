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
    uint16_t    entry1;
    uint16_t    entry2;
} Rec;

Rec tab[6] = {
    { 0x1234, 0x3456 },
    { 0x5678, 0x7890 },
    { 0x90AB, 0xABCD },
    { 0xCDEF, 0xEFDC },
    { 0xDCBA, 0xBA09 },
    { 0x0987, 0x8765 }
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
