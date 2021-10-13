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

#define CALADC10_15V_30C  *((unsigned int *)0x1A1A)   // Temperature Sensor Calibration-30 C
                                                      //See device datasheet for TLV table memory mapping
#define CALADC10_15V_85C  *((unsigned int *)0x1A1C)   // Temperature Sensor Calibration-85 C


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

volatile long temp;
volatile long IntDegF;
volatile long IntDegC;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  LEDInit();
  // Configure ADC10 - Pulse sample mode; ADC10SC trigger
  ADC10CTL0 = ADC10SHT_8 + ADC10ON;         // 16 ADC10CLKs; ADC ON,temperature sample period>30us
  ADC10CTL1 = ADC10SHP + ADC10CONSEQ_0;     // s/w trig, single ch/conv
  ADC10CTL2 = ADC10RES;                     // 10-bit conversion results
  ADC10MCTL0 = ADC10SREF_1 + ADC10INCH_10;  // ADC input ch A10 => temp sense

  // Configure internal reference
  while(REFCTL0 & REFGENBUSY);              // If ref generator busy, WAIT
  REFCTL0 |= REFVSEL_0+REFON;               // Select internal ref = 1.5V
                                            // Internal Reference ON
  ADC10IE |=ADC10IE0;                       // enable the Interrupt request for a completed ADC10_B conversion

  __delay_cycles(400);                      // Delay for Ref to settle


  //configure UART
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

  while(1)
  {
    UCA0TXBUF = 'c';
    ADC10CTL0 |= ADC10ENC + ADC10SC;        // Sampling and conversion start

    __bis_SR_register(LPM4_bits + GIE);     // LPM4 with interrupts enabled
    __no_operation();

    // Temperature in Celsius
    // The temperature (Temp, °„C)=
    IntDegC = (temp - CALADC10_15V_30C) *  (85-30)/(CALADC10_15V_85C-CALADC10_15V_30C) +30;
    UCA0TXBUF = temp;
    // Temperature in Fahrenheit
    // Tf = (9/5)*Tc + 32
    IntDegF = 9*IntDegC/5+32;

    __no_operation();                       // SET BREAKPOINT HERE

  }
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


// ADC10 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(ADC10IV,12))
  {
    case  0: break;                          // No interrupt
    case  2: break;                          // conversion result overflow
    case  4: break;                          // conversion time overflow
    case  6: break;                          // ADC10HI
    case  8: break;                          // ADC10LO
    case 10: break;                          // ADC10IN
    case 12: temp = ADC10MEM0;
             __bic_SR_register_on_exit(LPM4_bits);
             break;                          // Clear CPUOFF bit from 0(SR)
    default: break;
  }
}

