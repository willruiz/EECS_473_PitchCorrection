#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <stdint.h>

// Used https://os.mbed.com/users/4180_1/notebook/using-a-microphone-for-audio-input/ for help & inspiration

// I2S microphone here has its own library : https://os.mbed.com/users/p07gbar/code/I2S/log/455d5826751b/I2S.h/

// possibly useful, haven't read entirely: https://www.analog.com/media/en/dsp-documentation/embedded-media-processing/embedded-media-processing-chapter5.pdf

// Microphone Interface

enum Note {A, As, B, C, Cs, D, Ds, E, F, Fs, G, Gs}; // 's' indicates sharp
Note curr_note;
uint8_t curr_octave; 
float perc_error;
  
// initialize the pin to be used with the microphone.
void microphone_init(uint8_t LRCL_pin, uint8_t DOUT_pin, uint8_t BCLK_pin) {
  // initalize i2s connection with these pins
  // TODO: change this config to fit your MEMS microphone and audio processing needs.
	nrfx_i2s_config_t config =
		NRFX_I2S_DEFAULT_CONFIG(BCLK_pin, LRCL_pin,
								NRFX_I2S_PIN_NOT_USED,
								NRFX_I2S_PIN_NOT_USED, DOUT_pin); // NRFX_I2S_DEFAULT_CONFIG(_pin_sck, _pin_lrck, _pin_mck, _pin_sdout, _pin_sdin)

	config.mode = NRF_I2S_MODE_MASTER;			//Microphone requirement
	config.ratio = NRF_I2S_RATIO_64X;			//Microphone requirement
	config.sample_width = NRF_I2S_SWIDTH_24BIT_IN32BIT; //Microphone requirement
	config.mck_setup = NRF_I2S_MCK_32MDIV31;	//Preference     freq = (MCKfreq/ratio) =16.129 KHz.
	config.channels = NRF_I2S_CHANNELS_LEFT;	//Preference
}

// read in the current value of the microphone, use in interrrupts
float microphone_read() {
   // initiate i2s recieve 
}
  
// tuning, error checking, note identifying algorithm
// assign curr_note, perc_error for output to haptic motors and/or screen
void convert();



#endif
