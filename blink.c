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


volatile long temp;
volatile long IntDegF;
volatile long IntDegC;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

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


  while(1)
  {
    ADC10CTL0 |= ADC10ENC + ADC10SC;        // Sampling and conversion start

    __bis_SR_register(LPM4_bits + GIE);     // LPM4 with interrupts enabled
    __no_operation();

    // Temperature in Celsius
    // The temperature (Temp, °„C)=
    IntDegC = (temp - CALADC10_15V_30C) *  (85-30)/(CALADC10_15V_85C-CALADC10_15V_30C) +30;

    // Temperature in Fahrenheit
    // Tf = (9/5)*Tc + 32
    IntDegF = 9*IntDegC/5+32;

    __no_operation();                       // SET BREAKPOINT HERE

  }
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

