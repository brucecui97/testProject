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

enum NextByteType{START, COMMAND, DATA1, DATA2, ESCAPE, UNKNOWN};

Queue_t     q;  // Queue declaration
volatile Rec myRec;
volatile char commandByte;
volatile char dataByte1;
volatile char dataByte2;
volatile char escapeByte;
volatile uint16_t finalData;
volatile enum NextByteType nextByteType = UNKNOWN;
volatile int temp123;
volatile uint64_t  frequencyDesiredSet = 1000000;



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

    //cofigure LED

    //first four bits
    PJDIR |= BIT0;
    //Leds Off
    PJOUT &= ~BIT0;
    // Configure ports for UCA0
    P2SEL0 &= ~(BIT0 + BIT1);
    P2SEL1 |= BIT0 + BIT1;

       P3DIR |= BIT4;                       // P3.4
       P3SEL0 |= BIT4;                      // P3.4

       int desiredFrequency = 5000;

       int pwmPeriod = frequencyDesiredSet/desiredFrequency; //set to 1mhz/desiredFrequency
       TB1CCR0 = pwmPeriod;                         // PWM Period

       TB1CCTL1 = OUTMOD_7;                      // CCR1 reset/set
       TB1CCR1 = pwmPeriod/2;                            // CCR1 PWM duty cycle


       TB1CTL = TBSSEL_2 + MC_1 + TBCLR;         // SMCLK, up mode, clear TAR





    // Configure UCA0
    UCA0CTLW0 = UCSSEL0;
    UCA0BRW = 52;
    UCA0MCTLW = 0x4900 + UCOS16 + UCBRF0;
    UCA0IE |= UCRXIE;

    // global interrupt enable
    _EINT();

    while(1){
        Rec tempRec;
        q_peek(&q, &tempRec);
        if(q_peek(&q, &tempRec) && tempRec.entry1==START_BYTE && q.cnt>=5){

            q_pop(&q, &tempRec);//pop start Byte

            q_pop(&q, &tempRec);
            commandByte = tempRec.entry1;

            q_pop(&q, &tempRec);
            dataByte1 = tempRec.entry1;

            q_pop(&q, &tempRec);
            dataByte2 = tempRec.entry1;

            q_pop(&q, &tempRec);
            escapeByte = tempRec.entry1;

            if (escapeByte == 1){
                dataByte2 = 255;
            }
            else if (escapeByte == 2){
                dataByte1 = 255;
            }
            else if (escapeByte == 3){
                dataByte1 = 255;
                dataByte2 = 255;
            }
            finalData = (dataByte1 << 8) | (dataByte2 & 0xff);

           int desiredFrequency = finalData;

           int pwmPeriod = frequencyDesiredSet/desiredFrequency; //set to 1mhz/desiredFrequency

           TB1CCR0 = pwmPeriod;                         // PWM Period

           TB1CCTL1 = OUTMOD_7;                      // CCR1 reset/set
           TB1CCR1 = pwmPeriod/2;                            // CCR1 PWM duty cycle

           //do commands
           if (commandByte == 2){
              PJOUT|=BIT0;
           }
           else if (commandByte == 3){
               PJOUT&= ~BIT0;
           }

        }

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

