#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <assert.h>
#include "task.h"

void display_digit0_handler(void);
void display_digit1_handler(void);
void adc_handler(void);
void temp_handler(void);
void relay_handler(void);

static task_t tasks[] = {
    { &display_digit0_handler, .period = 256, .counter = 20 },
    { &display_digit1_handler, .period = 256, .counter = 180 },
    { &adc_handler, .period = 255, .counter = 0 },
    { &temp_handler, .period = 2048, .counter = 10 },
    { &relay_handler, .period = 2048, .counter = 0 }
};

static const uint8_t num_tasks = sizeof(tasks) / sizeof(task_t);

static volatile uint16_t ticks = 0;

/* 
 * Sets up the timing interrupt to occur at around 16 kHz
 */
void task_setup(void) {
    // Timer 1 CTC mode
	TCCR1B |= (1 << CTC1);

	// Prescale timer 1 clock by 1/8
    TCCR1B |= (1 << CS12);

	// Enable timer 1 compare match interrupt
	TIMSK |= (1 << OCIE1A);

    // Timer period. Gives an interrupt frequency of 16129 Hz
    OCR1A = OCR1C = 62;
  
	// enable interrupts
	sei();
}

void task_start(void) {
    TIMSK |= 1 << OCIE1A;
}

void task_stop(void) {
    TIMSK &= ~(1 << OCIE1A);
}

/* 
 * 16kHz (16025 Hz) timer interrupt
 */
ISR(TIMER1_CMPA_vect) {
    ticks++;
}

void task_manager(void) {
    uint16_t last_tick = 0;

    while (true) {
        // Wait until new tick arrives
        while (ticks == last_tick);

        // Loop through each task and update its count
        for (uint8_t i = 0; i < num_tasks; i++) {
            tasks[i].counter += ticks - last_tick;
        }

        last_tick = ticks;

        for (uint8_t i = 0; i < num_tasks; i++) {
            // If number of ticks changed after previous task, stop going further
            if (ticks != last_tick) {
                break;
            }

            // Call the task handler when the count reaches the period
            if (tasks[i].counter >= tasks[i].period) {
                tasks[i].counter = 0;
                tasks[i].handler();
            }
        }
    }
}
