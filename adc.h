#pragma once

#include <stdbool.h>
#include <stdint.h>

bool adc_ready(void);
uint16_t adc_read(void);
void adc_setup(void);
void adc_set_channel(uint8_t channel);
