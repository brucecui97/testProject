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
// The FRAM section from 0xD000 - 0xF000 is used by all modes
// for performing writes to FRAM
// Do not use this section for code or data placement.
// It will get overwritten!
#define ADC_START_ADD 0xD400
#define ADC_END_ADD 0xF000
#define FRAM_TEST_START 0xD400
#define FRAM_TEST_END 0xF000
#define MEM_UNIT 0x200

// Pin Definitions
#define ACC_PWR_PIN       BIT7
#define ACC_PWR_PORT_DIR  P2DIR
#define ACC_PWR_PORT_OUT  P2OUT
#define ACC_PORT_DIR      P3DIR
#define ACC_PORT_OUT      P3OUT
#define ACC_PORT_SEL0     P3SEL0
#define ACC_PORT_SEL1     P3SEL1
#define ACC_X_PIN         BIT0
#define ACC_Y_PIN         BIT1
#define ACC_Z_PIN         BIT2

// Accelerometer Input Channel Definitions
#define ACC_X_CHANNEL     ADC10INCH_12
#define ACC_Y_CHANNEL     ADC10INCH_13
#define ACC_Z_CHANNEL     ADC10INCH_14

volatile unsigned int x_acc = 0;
volatile unsigned int y_acc = 0;
volatile unsigned int z_acc = 0;
void SetupAccel(void)
{
  //Setup  accelerometer
  // ~20KHz sampling
  //Configure GPIO
  ACC_PORT_SEL0 |= ACC_X_PIN + ACC_Y_PIN + ACC_Z_PIN;    //Enable A/D channel inputs
  ACC_PORT_SEL1 |= ACC_X_PIN + ACC_Y_PIN + ACC_Z_PIN;
  ACC_PORT_DIR &= ~(ACC_X_PIN + ACC_Y_PIN + ACC_Z_PIN);
  ACC_PWR_PORT_DIR |= ACC_PWR_PIN;              //Enable ACC_POWER
  ACC_PWR_PORT_OUT |= ACC_PWR_PIN;

  // Allow the accelerometer to settle before sampling any data
  __delay_cycles(2000);

  //Single channel, once,
  ADC10CTL0 &= ~ADC10ENC;                        // Ensure ENC is clear
  ADC10CTL0 = ADC10ON + ADC10SHT_5;
  ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
  ADC10CTL2 = ADC10RES;
  ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;
  ADC10IV = 0x00;                          // Clear all ADC12 channel int flags
  ADC10IE |= ADC10IE0;
}

void TakeADCMeas(void)
{
  while (ADC10CTL1 & BUSY);
  ADC10CTL0 |= ADC10ENC | ADC10SC ;       // Start conversion
  __bis_SR_register(CPUOFF + GIE);        // LPM0, ADC10_ISR will force exit
  __no_operation();                       // For debug only
}

volatile unsigned int ADCResult = 0;
int main(void){
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    // Configure clocks and timer
    CSCTL0 = 0xA500;                        // Write password to modify CS registers
    CSCTL1 = DCOFSEL0 + DCOFSEL1;           // DCO = 8 MHz
    CSCTL2 = SELM0 + SELM1 + SELA0 + SELA1 + SELS0 + SELS1; // MCLK = DCO, ACLK = DCO, SMCLK = DCO

    int pwmPeriod = 12400;
    TB1CCR0 = pwmPeriod;                         // PWM Period

    TB1CCTL1 = OUTMOD_7 + CCIE;                      // CCR1 reset/set
    TB1CCR1 = pwmPeriod/2;                            // CCR1 PWM duty cycle
    TB1CTL = TBSSEL_2 + MC_1 + TBCLR;         // SMCLK, up mode, clear TAR
    P1DIR |= BIT0;
    P1OUT |= BIT0;

    SetupAccel();


    while(1){
        //loop through x y z axis to get the values and store them in global variable
        //setup new ADC on different port

        ADC10CTL0 &= ~ADC10ENC;                        // Ensure ENC is clear
        ADC10CTL0 = ADC10ON + ADC10SHT_5;
        ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
        ADC10CTL2 = ADC10RES;
        ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_12;
        ADC10IV = 0x00;                          // Clear all ADC12 channel int flags
        ADC10IE |= ADC10IE0;
        TakeADCMeas();
        x_acc = ADCResult>>2;

        ADC10CTL0 &= ~ADC10ENC;                        // Ensure ENC is clear
        ADC10CTL0 = ADC10ON + ADC10SHT_5;
        ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
        ADC10CTL2 = ADC10RES;
        ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_13;
        ADC10IV = 0x00;                          // Clear all ADC12 channel int flags
        ADC10IE |= ADC10IE0;
        TakeADCMeas();
        y_acc = ADCResult>>2;

        ADC10CTL0 &= ~ADC10ENC;                        // Ensure ENC is clear
        ADC10CTL0 = ADC10ON + ADC10SHT_5;
        ADC10CTL1 = ADC10SHS_0 + ADC10SHP + ADC10CONSEQ_0 + ADC10SSEL_0;
        ADC10CTL2 = ADC10RES;
        ADC10MCTL0 = ADC10SREF_0 + ADC10INCH_14;
        ADC10IV = 0x00;                          // Clear all ADC12 channel int flags
        ADC10IE |= ADC10IE0;
        TakeADCMeas();
        z_acc = ADCResult>>2;
    }
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  switch(__even_in_range(ADC10IV,ADC10IV_ADC10IFG))
  {
    case ADC10IV_NONE: break;               // No interrupt
    case ADC10IV_ADC10OVIFG: break;         // conversion result overflow
    case ADC10IV_ADC10TOVIFG: break;        // conversion time overflow
    case ADC10IV_ADC10HIIFG: break;         // ADC10HI
    case ADC10IV_ADC10LOIFG: break;         // ADC10LO
    case ADC10IV_ADC10INIFG: break;         // ADC10IN
    case ADC10IV_ADC10IFG:
             ADCResult = ADC10MEM0;
             __bic_SR_register_on_exit(CPUOFF);
             break;                          // Clear CPUOFF bit from 0(SR)
    default: break;
  }
}

#pragma vector = TIMER1_B1_VECTOR
__interrupt void Timer_B (void)
{
  P1OUT ^= BIT0;
  TB1CCTL1 &= ~CCIFG;
}


