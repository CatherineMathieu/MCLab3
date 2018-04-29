//*****************************************************************
// MC Lab 3
//*****************************************************************

// Headerfile Tiva
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "tm4c1294ncpdt.h"

void configSys(void);
void displayValue();


int main() {

    configSys();
    displayValue(5000);
}


//Develop the function configSys() which configures the ports
// (and timer if you want to realize the time delay by a timer):

void configSys(void){
    SYSCTL_RCGCGPIO_R |= 0x00000E09;            // Enables PK(AIN0 to AIN7 belong to Port K), PL, PM, PD
    while(!(SYSCTL_RCGCGPIO_R & 0x00000E09));   // Wait for PK, PL, PM & PD Ready flags

    SYSCTL_RCGCADC_R |= (1<<0); // ADC0 digital block
    while(!(SYSCTL_PRADC_R & 0x01)); // Ready ?

    // configure AIN0 to AIN7 (= PK(7..0)) as analog inputs
    GPIO_PORTK_AFSEL_R |=0xFF; // PK7..0 Alternative Pin Function enable
    GPIO_PORTK_DEN_R &= ~0xFF; // PK7..0 disable digital IO
    GPIO_PORTK_AMSEL_R |= 0x0FF; // PK7..0 enable analog function
    GPIO_PORTK_DIR_R &= ~0x00; // Allow Outputs PK7..0

    // ADC0_SS0 configuration
    ADC0_ACTSS_R &= ~0x0F; // disable all 4 sequencers of ADC0
    //Magic code
    SYSCTL_PLLFREQ0_R |= (1<<23); // PLL Power
    while (!(SYSCTL_PLLSTAT_R & 0x01)); // until PLL has locked
    ADC0_CC_R |= 0x01; waitcycle++; // PIOSC for ADC sampling clock
    SYSCTL_PLLFREQ0_R &= ~(1<<23); // PLL Power off

    ADC0_SSMUX0_R |= 0x01234567; // sequencer 0, channel AIN7 down to AIN0
    ADC0_SSCTL0_R |= 0x20000000; // END7 = 1 set, sequence length = 8
    ADC0_ACTSS_R |= 0x01; // enable sequencer 0 ADC0

    GPIO_PORTD_AHB_DIR_R |= 0x00000000;             // Set Port D Pin 0 and 1 to Input
    GPIO_PORTD_AHB_DEN_R |= 0x00000011;             // Enable Port D Pin 0 and 1

    GPIO_PORTM_DIR_R |= 0x000000FF;             // Set Port M Pins 7-0 to Outputs
    GPIO_PORTM_DEN_R |= 0x000000FF;             // Enable Port M Pins 7-0

    GPIO_PORTL_DIR_R |= 0x00000007;             // Set Port L Pin 0 to 2 to Outputs
    GPIO_PORTL_DEN_R |= 0x00000007;             // Enable Port L Pin 0 to 2
}

//Develop the function displayValue() that receives the successive approximations
// register value and calculates the voltage in mV. (Full range is 5V, the
// resolution 10 bit, see assignment sheet for details). Use integer (fix point)
// operations instead of floating point. Afterwards the function outputs the
// value in mV on the 7-segment display. For this, the voltage value (mV) has
// to be decomposed into decimal digits. Write the function in C
// (incl . conversion to mV and output to display).

void displayValue(int mV){
    ADC0_ACTSS_R |= 0x01; // enable sequencer ADC0_SS0

    while (1) {

        ADC0_PSSI_R |= 0x01; // start ADC0_SS0
        while (ADC0_SSFSTAT0_R & (1 << 8)) // wait for FIFO non-empty
            ;
        // read ADC value from FIFO, convert in mV and display
        ulData = (unsigned long) ADC0_SSFIFO0_R * 5000UL / 4095;
        printf(" %04d\n", ulData);
}

//Write the program for task 2.2.1 which includes the configuration of the ADC
// and the sampling of channel PE(7).
