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
uint32_t recieve_buffer[WORDS_IN_RX_BLOCK]; // how many words to receive at one time??
nrfx_i2s_buffers_t initial_buffers;

// data_handler  
static void data_handler(nrfx_i2s_buffers_t const *p_released, u32_t status)
{
	if (NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED == status)
	{
		nrfx_err_t err = nrfx_i2s_next_buffers_set(&initial_buffers);
		if (err != NRFX_SUCCESS)
		{
			printk("Error!, continuing running as if nothing happened, but you should probably investigate.\n");
		}
	}
	if (p_released)
	{
		if (p_released->p_rx_buffer != NULL)
		{
			data_ready_flag = true; 
		}
	}
}

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
	config.sample_width = NRF_I2S_SWIDTH_24BIT_IN32BIT; 	//Microphone requirement
	
	// TODO: possibly change for feather
	config.mck_setup = NRF_I2S_MCK_32MDIV31;	//Preference     freq = (MCKfreq/ratio) =16.129 KHz.
	config.channels = NRF_I2S_CHANNELS_LEFT;	//Preference
	
	nrfx_err_t err_code = nrfx_i2s_init(&config, data_handler);
	if (err_code != NRFX_SUCCESS)
	{
		printk("I2S init error\n");
		return err_code;
	}
}

// read in the current value of the microphone, call this fxn in interrrupts
float microphone_read() {
   // initiate i2s recieve 
	memset(&recieve_buffer, 0x00, sizeof(recieve_buffer));
	initial_buffers->p_rx_buffer = recieve_buffer;
	
	err_code = nrfx_i2s_start(&initial_buffers, I2S_DATA_BLOCK_WORDS, 0); //start recording
	if (err_code != NRFX_SUCCESS)
	{
		printk("I2S start error\n");
		return err_code;
	}
	//nrfx_i2s_stop() //stop recording
	
}
  
// tuning, error checking, note identifying algorithm
// assign curr_note, perc_error for output to haptic motors and/or screen
void convert();



#endif
