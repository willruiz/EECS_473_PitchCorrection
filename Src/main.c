#include <zephyr.h>
#include <hal/nrf_gpio.h>

#include "microphone.h"
#include "pitchdetection.h"
#include "haptic_feedback.h"

// Selected pins
#define I2S_LRCK_PIN 4
#define I2S_DIN_PIN 5
#define I2S_BCLK_PIN 2
#define I2S_BCLK_PWM_PIN 29
#define I2S_LRCK_PWM_PIN 28
#define HAPTIC_LEFT_PIN 11
#define HAPTIC_RIGHT_PIN 31

// The uncertainty at which we no longer trust the data
#define UNCERTAINTY_DISCARD 0.10f
#define DROP_UNCERTAIN_DATA 1 // Change this to zero to turn dropping off
#define UNCERTAINTY_COUNT_TO_DISABLE 10 //The number of uncertain reads in a row before haptic feedback is disabled

#define CUTOFF_ERROR 5.f //The allowed error band

// Debug LED pins
#define DBG_PIN_LED2 12
#define DBG_PIN_LED3 13
#define DBG_PIN_19 6
#define DBG_PIN_20 7
#define DBG_PIN_21 8
#define DBG_PIN_28 14
#define DBG_PIN_29 15

// Debug pin for latency testing (PROTOTYPE)
#define DEBUG_LATENCY_PIN DBG_PIN_29
#define DO_LATENCY_TESTING 0

//LED debugging
#define DO_LED_DEBUGGING 1

// Main variables
float frequency;
float uncertainty;
uint16_t uncertainty_counter;
const char *note;
float error;
uint8_t i;

// Buffer for averaging
#define BUF_AVG_SIZE 1
float freqs_avg_buffer[BUF_AVG_SIZE];
uint8_t freq_curr_index = 0;

int main() {

	// // Hello world PCB!
	// #define DEBUG_PIN DBG_PIN_29
	// #define DEBUG_LED DBG_PIN_LED2
	// nrf_gpio_cfg_output(DEBUG_PIN);
	// nrf_gpio_cfg_output(DEBUG_LED);
	// nrf_gpio_cfg_input(DBG_PIN_29, NRF_GPIO_PIN_PULLDOWN);
	// nrf_gpio_cfg_output(I2S_BCLK_PWM_PIN);
	// nrf_gpio_pin_set(I2S_BCLK_PWM_PIN);
	// while(1) {
	// 	nrf_gpio_pin_clear(DEBUG_PIN);
	// 	nrf_gpio_pin_clear(DEBUG_LED);
	// 	k_sleep(K_MSEC(500));
	// 	if (nrf_gpio_pin_read(DBG_PIN_29)) {
	// 		nrf_gpio_pin_set(DEBUG_PIN);
	// 		nrf_gpio_pin_set(DEBUG_LED);
	// 		while(1) {}
	// 	}
	// 	k_sleep(K_MSEC(500));
	// }


	// Initialize microphone
	microphone_init(I2S_LRCK_PIN,
					I2S_DIN_PIN,
					I2S_BCLK_PIN,
					I2S_BCLK_PWM_PIN,
					I2S_LRCK_PWM_PIN);

	// Initialize pitch detection audio buffer
	set_pitch_buffer(get_audio_buffer());

	// Initialize motor output
	haptic_init(HAPTIC_LEFT_PIN, HAPTIC_RIGHT_PIN);

	// Optionally initialize latency debugging pin
	if (DO_LATENCY_TESTING) {
		nrf_gpio_cfg_output(DEBUG_LATENCY_PIN);
	}

	// Optionally initialize led debugging pins
	if (DO_LED_DEBUGGING) {
		nrf_gpio_cfg_output(DBG_PIN_LED2);
		nrf_gpio_cfg_output(DBG_PIN_LED3);
	}

	// Start microphone
	microphone_start();

	// Haptic motor testing code for milestone 1
	// uint8_t max = 0;
	// while(1) {
	// 	haptic_set(HAPTIC_LEFT, HAPTIC_ENABLED, (float)max / 255.f);
	// 	k_sleep(K_MSEC(100));
	// 	max = (max + 1) % 255;
	// 	printk("%d\n", max);
	// }

	// // Microphone testing code for milestone 1
	// int32_t *audio_buf = get_audio_buffer();
	// uint16_t i;
	// while(1) {
	// 	for(i = 0; i < BUF_SIZE; i++) {
	// 		printk("%d,\n", audio_buf[i]);
	// 	}
	// }

	while(1) {

		// Latency testing code
		if (DO_LATENCY_TESTING) {
			nrf_gpio_pin_set(DEBUG_LATENCY_PIN);
		}

		//LED debugging
		if (DO_LED_DEBUGGING) {
			if (uncertainty > UNCERTAINTY_DISCARD /*(error > 0 && error < 30.f) || (error < 0 && error > -30.f)*/) {
				nrf_gpio_pin_clear(DBG_PIN_LED3);
			} else {
				nrf_gpio_pin_set(DBG_PIN_LED3);
			}
		}


		// Perform frequency calculations
		frequency = predict_freq(&uncertainty);
		if (DROP_UNCERTAIN_DATA && uncertainty > UNCERTAINTY_DISCARD) {
			//TODO add code here to disable the motors if we are uncertain for too long
			if (uncertainty_counter++ >= UNCERTAINTY_COUNT_TO_DISABLE) {
				haptic_set_both_enable(HAPTIC_DISABLED);
				//printk("Uncertain for too long. Haptic feedback disabled.\n");
			}

			if (DO_LATENCY_TESTING) {
				nrf_gpio_pin_clear(DEBUG_LATENCY_PIN);
				k_sleep(K_SECONDS(1));
			}

			if (DO_LED_DEBUGGING) {
				nrf_gpio_pin_clear(DBG_PIN_LED2);
			}
			//printk("Uncertainty drop. Freq %f, Uncertainty %f\n", frequency, uncertainty * 100.f);
			continue;
		}

		//We have a not-uncertain frequency
		uncertainty_counter = 0;
			
		freqs_avg_buffer[freq_curr_index] = frequency;
		freq_curr_index = (freq_curr_index + 1) % BUF_AVG_SIZE;

		// Average the past BUF_AVG_SIZE frequencies
		frequency = 0;
		for(i = 0; i < BUF_AVG_SIZE; i++) {
			frequency += freqs_avg_buffer[i];
		}
		frequency /= BUF_AVG_SIZE;

		// Find matching note and error
		note = find_closest_note(frequency, &error);

		// Set motor output to reflect error
		if (error > -CUTOFF_ERROR && error < CUTOFF_ERROR) {
			haptic_set_both_enable(HAPTIC_DISABLED);
			if (DO_LED_DEBUGGING) {
				nrf_gpio_pin_set(DBG_PIN_LED2);
			}
		} else if (error < 0) {
			haptic_set(HAPTIC_LEFT, HAPTIC_ENABLED, error / -100.f); //Left is flat
			haptic_set(HAPTIC_RIGHT, HAPTIC_DISABLED, 0.f);
			if (DO_LED_DEBUGGING) {
				nrf_gpio_pin_clear(DBG_PIN_LED2);
			}
		} else {
			haptic_set(HAPTIC_RIGHT, HAPTIC_ENABLED, error / 100.f); //Right is sharp
			haptic_set(HAPTIC_LEFT, HAPTIC_DISABLED, 0.f);
			if (DO_LED_DEBUGGING) {
				nrf_gpio_pin_clear(DBG_PIN_LED2);
			}
		}

		// Latency testing code
		if (DO_LATENCY_TESTING) {
			nrf_gpio_pin_clear(DEBUG_LATENCY_PIN);
			k_sleep(K_SECONDS(1));
		}
		
		//printk("Predicted note: %s,\t\tfrequency: %f,\t\terror: %f,\t\tuncertainty: %f\n", note, frequency, error, uncertainty * 100.f);
	}

}