#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <stdint.h>
#include "nrfx_i2s.h"

// Pitch detection tunable constants
#define SAMPLE_RATE 62500           // The sample rate of the microphone at the given PWM clock frequency
#define MAX_FREQUENCY 4000          // The maximum frequency we check for
#define MIN_FREQUENCY 130           // The minimum frequency we check for
#define K 0.98                      // The threshold constant in the computing algorithm (see paper)

// Derived constants
#define MIN_TAU SAMPLE_RATE / MAX_FREQUENCY
#define MAX_TAU SAMPLE_RATE / MIN_FREQUENCY
#define BUF_SIZE (uint16_t)((MAX_TAU) * 2)
  
// Initialize the microphone driver. Pass BLCK and LRCL PWM pins that should be connected to the clock pins on both the microphone and the nRF. See implementation for details.
nrfx_err_t microphone_init(uint8_t LRCL_pin, uint8_t DIN_pin, uint8_t BCLK_pin, uint8_t BLCK_PWM_pin, uint8_t LRCL_PWM_pin);

// Stops reading microphone data.
void microphone_stop(void);
  
// Starts reading microphone data.
nrfx_err_t microphone_start(void);

int32_t *get_audio_buffer();

#endif
