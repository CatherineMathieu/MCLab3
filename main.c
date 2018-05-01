//*****************************************************************
// MC Lab 3
// Autor: Catherine Mathieu
//
//Objectives
//  Learn how successive approximation works using an external ADC circuit
//  Learn how to program the internal ADCof the Tiva TM4C1294 microcontroller.
//  Learn how to drive a 7seg display by a microcontroller and convert binary numbers to BCD numbers
//  Learn to build a digital voltmeter with an ADC and a microncontroller

//Assignments
//  Download the assignment here (link to EMIL, login required): Nr.1-DigVoltmeter (bilingual doc) or
//      https://www.elearning.haw-hamburg.de/course/view.php?id=217
//  Please do the following two assignments only:
//  Exercise 2: Weighting conversion method (sucessive approximation)
//  Exercise 3: Internal A/D converter
//*****************************************************************

// Headerfile Tiva
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "tm4c1294ncpdt.h"

void configSys(void);
void displayValue(unsigned);
void timerConfig(void);
void timerWait(unsigned short usec);
void configADC();


int main() {

    configSys();
    //configADC(); //Exercice 3

    while(1)
    {
        while(GPIO_PORTD_AHB_DATA_R &= ~0x02) // While PD(1) is false, the AD conversion is working
        {
            for(int i = 7; i <= 0; i--)
            {
                GPIO_PORTK_DATA_R |= (1 << i); // Test each bit pattern keeping the previous result

                if (GPIO_PORTD_AHB_DATA_R < 0x01)  // output voltage is higher than input voltage.
                {
                    GPIO_PORTK_DATA_R &= ~(1 << i); // the bit has to be clear.
                }

                timerWait(30);
            }
            displayValue(GPIO_PORTK_DATA_R);
        }
    }
}

//Develop the function configSys() which configures the ports
// (and timer if you want to realize the time delay by a timer):

void configSys(void)
{
    SYSCTL_RCGCGPIO_R |= 0x00000E08;           // Enables PK(AIN0 to AIN7 belong to Port K), PL, PM, PD
    while(!(SYSCTL_RCGCGPIO_R & 0x00000E08));  // Wait for PK, PL, PM & PD Ready flags

    GPIO_PORTD_AHB_DIR_R &= ~0x00000003;       // Set Port D Pins 0 and 1 to Input
    GPIO_PORTK_DIR_R |= 0x000000FF;            // Set Port K Pins 7-0 to Outputs
    GPIO_PORTL_DIR_R |= 0x00000007;            // Set Port L Pins 2-0 to Outputs
    GPIO_PORTM_DIR_R |= 0x000000FF;            // Set Port M Pins 7-0 to Outputs

    GPIO_PORTD_AHB_DEN_R |= 0x00000003;         // Enable Port D Pins 0 and 1
    GPIO_PORTK_DEN_R |= 0x000000FF;             // Enable Port K Pins 7-0
    GPIO_PORTL_DEN_R |= 0x00000007;             // Enable Port L Pins 2-0
    GPIO_PORTM_DEN_R |= 0x000000FF;             // Enable Port M Pins 7-0

    timerConfig();
}

/*Develop the function displayValue() that receives the successive approximations
register value and calculates the voltage in mV. (Full range is 5V, the
resolution 10 bit, see assignment sheet for details). Use integer (fix point)
operations instead of floating point. Afterwards the function outputs the
value in mV on the 7-segment display. For this, the voltage value (mV) has
to be decomposed into decimal digits. Write the function in C
(incl . conversion to mV and output to display).*/

void displayValue(unsigned SAR){

    float voltageMax = 500;
    int voltageResult = 0;
    int digit3 = 0;
    int digit2 = 0;
    int digit1 = 0;

    voltageResult = (float)SAR / 0xFF * voltageMax;
    digit1 = voltageResult % 10;
    digit2 = voltageResult / 10 % 10;
    digit3 = voltageResult / 100;

    GPIO_PORTL_DATA_R = 0x00;           // Enable PL(0)
    while(!(GPIO_PORTL_DATA_R & 0x00)); // Wait for PL(0) is ready
    GPIO_PORTM_DATA_R = (digit2 << 4) | digit1; //Display two last number

    GPIO_PORTL_DATA_R = 0x01;           // Enable PL(0)
    while(!(GPIO_PORTL_DATA_R & 0x01)); // Wait for PL(1) is ready
    GPIO_PORTM_DATA_R = digit3;         //Display first number
}

void timerConfig(void)
{
    SYSCTL_RCGCTIMER_R |= (1<<0);      // Activate Timer0 Clock
    while(!(SYSCTL_PRTIMER_R & 0x01)); // Wait for Timer0 Clock Ready flag
    TIMER0_CTL_R |= 0x00;              // Disable (stop) Timer0
    TIMER0_CFG_R = 0x00000004;         // Set Timer0 to 16 bit mode
    TIMER0_TAMR_R |= 0x00000001;       // Configure Timer0A Mode:
        // Disable Match, Set to Count down, Set to one shot, Enable Time-Out interrupt
    //TIMER0_TAPR_R = 1-1;               // no need of a Prescaler
}

void timerWait(unsigned short usec)
{
    if (usec <= 0) return;          // Error Handling

    uint32_t loadValue = (uint32_t)ceil(16 / 1 * usec) - 1;     // Calculate Load value

    TIMER0_TAILR_R = loadValue;     // Set Timer0A Interval Load Value
    TIMER0_CTL_R |= (1<<0);         // Enable (start) Timer0A
    while (!(TIMER0_RIS_R & 0x01)); // Poll Time-out flag
    TIMER0_ICR_R |= (1<<0);         // Clear interrupt flags
    TIMER0_CTL_R = 0x00;            // Disable (stop) Timer0
}

/*Write the program for task 2.2.1 which includes the configuration of the ADC
and the sampling of channel PE(7).

 The A/D conversion transforms the analog input voltage UE at pin PE(0) (= AIN3) into an
according binary value. The resulting binary number is proportional to UE. The voltage has to
be output with four digits using ports L and M.

PL(2) is use as trigger source from an oscilloscope.
 */

void configADC()
{
    SYSCTL_RCGCGPIO_R |= (1<<4);        // PE (AIN3 belong to Port E)
    while(!(SYSCTL_PRGPIO_R & 0x10));   // Ready ?

    SYSCTL_RCGCADC_R |= (1<<0);         // ADC0 digital block
    while(!(SYSCTL_PRADC_R & 0x01));    // Ready ?

    // configure AIN3 (= PE(3)) as analog inputs
    GPIO_PORTE_AHB_AFSEL_R |=0x08;      // PE3 Alternative Pin Function enable
    GPIO_PORTE_AHB_DEN_R &= ~0x08;      // PE3 disable digital IO
    GPIO_PORTE_AHB_AMSEL_R |= 0x08;     // PE3 enable analog function
    GPIO_PORTE_AHB_DIR_R &= ~0x08;      // Allow Input PE3

    // ADC0_SS0 configuration --- MAGIC CODE
    ADC0_ACTSS_R &= ~0x0F;              // disable all 4 sequencers of ADC0
    SYSCTL_PLLFREQ0_R |= (1<<23);       // PLL Power
    while (!(SYSCTL_PLLSTAT_R & 0x01)); // until PLL has locked
    ADC0_CC_R |= 0x01;
    // waitcycle++;                     // PIOSC for ADC sampling clock
    SYSCTL_PLLFREQ0_R &= ~(1<<23);      // PLL Power off
    ADC0_SSMUX0_R |= 0x00000003;        // sequencer 0, channel AIN3 only
    ADC0_SSCTL0_R |= 0x00000200;        // END1 set, sequence length = 1
    ADC0_ACTSS_R |= 0x01;               // enable sequencer 0 ADC0
}