#include <msp430.h>

#define ARRAY_SIZE 16
#define K 300 // proportional control

// variables for sending encoder data through UART
volatile unsigned int interrupt_counter = 0;
volatile unsigned int encoderA;
volatile unsigned int encoderB;
volatile unsigned int encoderA_prev = 0;
volatile unsigned int encoderB_prev = 0;

// variables for position control
volatile signed int current_position;
volatile signed int desired_position = 0;
volatile unsigned int encoderA_overflow = 0;
volatile unsigned int encoderB_overflow = 0;

// variables for circular buffer
volatile unsigned int start_index;
volatile unsigned int end_index;
volatile unsigned int count;
unsigned char circ_buffer[ARRAY_SIZE];

signed int get_position(void){
    signed int position_temp;
    position_temp = circ_buffer[start_index + 1];
    position_temp = position_temp << 8;
    position_temp |= circ_buffer[start_index + 2];

    return position_temp;
}

/*
void set_direction(unsigned char direction){
    switch (direction){
    case 1: // CW
        P3OUT |= BIT7;
        P3OUT &= ~BIT6;
        break;
    case 2: // CCW
        P3OUT |= BIT6;
        P3OUT &= ~BIT7;
        break;
    }
}
*/

void send_error(const char *str){

    while (*str != '\0') {
        /* Wait for the transmit buffer to be ready */
        while((UCA0IFG & UCTXIFG)==0);

        /* Transmit data */
        UCA0TXBUF = *str;

        str++;
    }

}

void remove_from_buffer(void){
    if (count == 0){
        send_error("No elements in buffer");
    }
    else{
//        while((UCA0IFG & UCTXIFG)==0);
//        UCA0TXBUF = circ_buffer[start_index];
        if (start_index == ARRAY_SIZE -1)
            start_index = 0;
        else
            start_index++;
        count--;
    }
}

void add_to_buffer(unsigned char byte_data){
    if (count == ARRAY_SIZE){
        remove_from_buffer();
        send_error("Buffer is full");
    }
    else{
        circ_buffer[end_index] = byte_data;
        if (end_index == ARRAY_SIZE - 1)
            end_index = 0;
        else
            end_index++;
        count++;
    }
}

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // circular buffer
    start_index = 0;
    end_index = 0;
    count = 0;

    // Configuring the clock
    CSCTL0_H = 0xA5; // clock register password
    CSCTL1 = DCORSEL + DCOFSEL0 + DCOFSEL1; // DCO at 24 MHz
    CSCTL2 = SELA_3 + SELS_3 + SELM_3; // ACLK = MCLK = SMCLK = DCO

    // Configuring pins for Encoder input to Timer A (P1.1, P1.2)
    P1DIR &= ~(BIT1 + BIT2);
    P1SEL0 &= ~(BIT1 + BIT2);
    P1SEL1 |= BIT1 + BIT2;

    // Configuring Timer A for Encoder input
    TA0CTL = TASSEL_0 + MC_2 + TAIE; // Select TA0CLK and continuous mode
    TA1CTL = TASSEL_0 + MC_2 + TAIE; // Select TA1CLK and continuous mode

    // TB2.2 pin for DC Motor PWM output (P2.2)
    P2DIR |= BIT2;
    P2SEL0 |= BIT2;
    P2SEL1 &= ~BIT2;

    // DC Motor direction pins (P3.6, P3.7)
    P3DIR |= BIT6 + BIT7;
    P3OUT |= BIT7;
    P3OUT &= ~BIT6; // set initially to CW

    // Configuring Timer TB2.2
    TB2CTL = TBSSEL_1 + MC_2; // select ACLK and continuous mode
    TB2CCR0 = 0xFFFF;
    TB2CCR2 = 0x0000; // set initially to 0 duty cycle
    TB2CCTL2 = OUTMOD_7; // reset/set mode

    // Configuring Timer TB2.1 for periodic interrupts
    TB2CCTL1 = CCIE;
    TB2CCR1 = 60000;

    // Configure UART
    UCA0CTLW0 |= UCSSEL_1; // ACLK
    UCA0BRW = 156; // baud rate 9600 with DCO = 24 MHz
    UCA0MCTLW = UCBRF_4 + UCOS16;

    UCA0CTLW0 &= ~UCSWRST; // reset the bit to enable communication
    UCA0IE = UCRXIE; // Interrupt enable for receive

    // Set up ports for UCA0 (P2.0, P2.1)
    P2SEL0 &= ~(BIT0 + BIT1);
    P2SEL1 |= BIT0 + BIT1;

    __enable_interrupt();

    volatile unsigned long duty_cycle;

    while(1){
        if (current_position == desired_position){
            TB2CCR2 = 0;
        }
        else if (current_position > desired_position){
            duty_cycle = (current_position - desired_position)*K;
            if (duty_cycle >= 0xFFFF) // cannot have duty cycle more than 100%
                duty_cycle = 0xFFFE;
//            else if (duty_cycle < 8000) // motor does not turn below this duty cycle
//                duty_cycle = 8000;
            TB2CCR2 = duty_cycle;
            P3OUT |= BIT7;
            P3OUT &= ~BIT6; // CW
        }
        else{
            duty_cycle = (desired_position - current_position)*K;
            if (duty_cycle >= 0xFFFF) // cannot have duty cycle more than 100%
                duty_cycle = 0xFFFE;
//            else if (duty_cycle < 8000) // motor does not turn below this duty cycle
//                duty_cycle = 8000;
            TB2CCR2 = duty_cycle;
            P3OUT |= BIT6;
            P3OUT &= ~BIT7; // CCW
        }

        if (count >= 3){
            if (circ_buffer[start_index] == 255){
                desired_position = get_position();
            }

            while(count != 0){
                remove_from_buffer();
            }
        }
    }

    //return 0;
}

#pragma vector = USCI_A0_VECTOR
__interrupt void RECEIVE_ISR(void){
    unsigned char RxByte;
    RxByte = UCA0RXBUF;

    // expecting 3 byte data package: start byte, signed MSB of position, signed LSB of position
    add_to_buffer(RxByte);
}

#pragma vector = TIMER2_B1_VECTOR // interrupt happening every 2.5 ms
__interrupt void TRANSMIT_ISR(void){
    TB2CCR1 += 60000;
    TB2CCTL1 &= ~CCIFG;

    interrupt_counter++;

    if (interrupt_counter == 16){ // sending through UART every 40 ms
        volatile unsigned int temp_byte;
        volatile unsigned int temp_encoderA;
        volatile unsigned int temp_encoderB;

        encoderA = TA1R;
        encoderB = TA0R;

        current_position = (encoderA + encoderA_overflow*0x10000) - (encoderB + encoderB_overflow*0x10000);

        if (encoderA_prev > encoderA)
            temp_encoderA = 0x10000 - encoderA_prev + encoderA;
        else
            temp_encoderA = encoderA - encoderA_prev;

        if (encoderB_prev > encoderB)
            temp_encoderB = 0x10000 - encoderB_prev + encoderB;
        else
            temp_encoderB = encoderB - encoderB_prev;

        while((UCA0IFG & UCTXIFG)==0);
        UCA0TXBUF = 255;

        while((UCA0IFG & UCTXIFG)==0);
        temp_byte = temp_encoderA >> 8;
        UCA0TXBUF = temp_byte;

        while((UCA0IFG & UCTXIFG)==0);
        UCA0TXBUF = temp_encoderA;

        while((UCA0IFG & UCTXIFG)==0);
        temp_byte = temp_encoderB >> 8;
        UCA0TXBUF = temp_byte;

        while((UCA0IFG & UCTXIFG)==0);
        UCA0TXBUF = temp_encoderB;

        interrupt_counter = 0;
        encoderA_prev = encoderA;
        encoderB_prev = encoderB;
    }

}

/* if encoder B (TA0R) has already overflowed, then reset encoderB_overflow
 * else set encoderA_overflow to 1
 */
#pragma vector = TIMER1_A1_VECTOR
__interrupt void ENCODERA_ISR(void){
    TA1CTL &= ~TAIFG;
//    if (encoderB_overflow == 1)
//        encoderB_overflow = 0;
//    else
//        encoderA_overflow = 1;
    encoderA_overflow++;
}

/* if encoder A (TA1R) has already overflowed, then reset encoderA_overflow
 * else set encoderB_overflow to 1
 */
#pragma vector = TIMER0_A1_VECTOR
__interrupt void ENCODERB_ISR(void){
    TA0CTL &= ~TAIFG;
//    if (encoderA_overflow == 1)
//        encoderA_overflow = 0;
//    else
//        encoderB_overflow = 1;
    encoderB_overflow++;
}
