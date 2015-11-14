#define F_CPU 8000000UL
#include <avr/io.h>
#include "adc.h"

void adc_setup(void) {
    // Set PA6 as input
    DDRA &= ~(1 << PA6);
    PORTA &= ~(1 << PA6);

    // Use ADC channel 5 (PA6)
    ADMUX |= 5 << MUX0;

    // Using internal 2.56V voltage reference
    ADMUX |= 1 << REFS1;

    // Prescale ADC timer by 1/128 CPU clock and enable ADC
    ADCSR |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
}

bool adc_ready(void) {
    return ! (ADCSR & (1 << ADSC));
}

void adc_set_channel(uint8_t channel) {
    ADMUX = (ADMUX & ~0b1111) | channel;
    ADCSR |= 1 << ADSC; // Start conversion
}

// Returns a value between 0 (0V) and 2000 (5V)
uint16_t adc_read(void) {
    uint16_t value = ADC;
    ADCSR |= 1 << ADSC; // Start conversion
    return value;
}
