#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <stdint.h>
#include "nrfx_i2s.h"
#include "nrfx_errors.h"

// Used https://os.mbed.com/users/4180_1/notebook/using-a-microphone-for-audio-input/ for help & inspiration

// I2S microphone here has its own library : https://os.mbed.com/users/p07gbar/code/I2S/log/455d5826751b/I2S.h/

// possibly useful, haven't read entirely: https://www.analog.com/media/en/dsp-documentation/embedded-media-processing/embedded-media-processing-chapter5.pdf

// Microphone Interface

#define WORDS_IN_RX_BLOCK 512


enum Note {A, As, B, C, Cs, D, Ds, E, F, Fs, G, Gs}; // 's' indicates sharp
  
// initialize the pin to be used with the microphone.
nrfx_err_t microphone_init(uint8_t LRCL_pin, uint8_t DOUT_pin, uint8_t BCLK_pin);

// read in the current value of the microphone, use in interrrupts
nrfx_err_t microphone_read(void);
  
// tuning, error checking, note identifying algorithm
// assign curr_note, perc_error for output to haptic motors and/or screen
void convert(void);



#endif
