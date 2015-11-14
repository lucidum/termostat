#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include "task.h"
#include "adc.h"

const uint8_t digits[] = {
    0b0111111, // 0
    0b0000110, // 1
    0b1011011, // 2
    0b1001111, // 3
    0b1100110, // 4
    0b1101101, // 5
    0b1111101, // 6
    0b0000111, // 7
    0b1111111, // 8
    0b1100111, // 9
    0b1111001, // E
    0b1110110, // H
};

#define ERROR 10
#define HIGH 11

volatile uint8_t number[2][3];
volatile uint8_t setpoint = 30;
volatile uint8_t relay_on = false;

void set_number(uint8_t display, uint8_t num, bool dp) {
    if (num > 99) {
        number[display][0] = HIGH;
        number[display][1] = HIGH;
        return;
    }

    number[display][0] = num / 10;
    number[display][1] = num % 10;
    number[display][2] = dp;
}

void set_error(uint8_t display) {
    number[display][0] = ERROR;
    number[display][1] = ERROR;
}

void render_digit(uint8_t digit, uint8_t value) {
    PORTA = (PORTA & ~0b11) | (1 << digit);
    PORTB = digits[value];
}

void render_dp(uint8_t value) {
    PORTA &= ~(1 << PA7);
    if (value) {
        PORTA |= 1 << PA7;
    }
}

void display_digit0_handler(void) {
    render_digit(0, number[0][0]);
}

void display_digit1_handler(void) {
    render_digit(1, number[0][1]);
    render_dp(number[0][2]);
}

typedef struct {
    unsigned int magnitude:16;
    unsigned int fraction:8;
} __attribute__((packed)) fixed_t;


fixed_t make_fixed(uint16_t number) {
    return (fixed_t) { number, 0 };
}

fixed_t fixed_div(fixed_t f, uint16_t div) {
    f.fraction = (uint32_t)(f.magnitude % div) * 256 / div + f.fraction / div;
    f.magnitude /= div;
    return f;
}

fixed_t fixed_add(fixed_t a, fixed_t b) {
    a.magnitude += b.magnitude + ((a.fraction + b.fraction) > 255);
    a.fraction += b.fraction;
    return a;
}

uint8_t fixed_round(fixed_t f) {
    return f.magnitude + (f.fraction >= 128);
}

#define N_SAMPLES 10
volatile fixed_t temp_samples[N_SAMPLES];
volatile uint8_t sample = 0;
volatile uint16_t raw_temp;
volatile uint8_t temp;

void temp_handler(void) {
    temp_samples[sample++] = make_fixed(raw_temp);

    if (sample == N_SAMPLES) {
        fixed_t t = make_fixed(0);
        for (uint8_t i = 0; i < N_SAMPLES; i++) {
            t = fixed_add(t, temp_samples[i]);
        }

        temp = fixed_round(fixed_div(fixed_div(t, N_SAMPLES), 4));
        set_number(0, temp, relay_on);
        sample = 0;
    }
}

void adc_handler(void) {
    if (adc_ready()) {
        raw_temp = adc_read();
    }

    //set_number(1, setpoint, relay_on);
}

void relay_handler(void) {
    if (relay_on) {
        if (temp + 0.25 < setpoint) {
            relay_on = false;
        }
    }
    else if (temp - 0.25 > setpoint) {
        relay_on = true;
    }

    PORTA = (PORTA & ~(1 << PA5)) | (relay_on << PA5);
}

int main(void) {
    DDRB |= 0b1111111;
    DDRA |= 0b11100001;
    PORTB = 0;

    adc_setup();
    task_setup();
    task_start();

    task_manager();
    return 0;
}
