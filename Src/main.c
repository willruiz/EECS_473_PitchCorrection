#include <zephyr.h>
#include <hal/nrf_gpio.h>

#include "microphone.h"
#include "pitchdetection.h"
#include "haptic_feedback.h"

// Selected pins
#define I2S_LRCK_PIN 4
#define I2S_DIN_PIN 5
#define I2S_BCLK_PIN 2
#define I2S_BCLK_PWM_PIN 28
#define I2S_LRCK_PWM_PIN 29
#define HAPTIC_LEFT_PIN 11
#define HAPTIC_RIGHT_PIN 31

// The uncertainty at which we no longer trust the data
#define UNCERTAINTY_DISCARD 0.5f
#define DROP_UNCERTAIN_DATA 1 // Change this to zero to turn dropping off

// Debug pin for latency testing
#define DEBUG_LATENCY_PIN 16
#define DO_LATENCY_TESTING 1

// Main variables
float frequency;
float uncertainty;
const char *note;
float error;
uint8_t i;

// Buffer for averaging
#define BUF_AVG_SIZE 10
float freqs_avg_buffer[BUF_AVG_SIZE];
uint8_t freq_curr_index = 0;

int main() {

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

	// Start microphone
	microphone_start();

	// // Haptic motor testing code for milestone 1
	// while(1) {
	// 	haptic_set(HAPTIC_LEFT, HAPTIC_ENABLED, 0.75f);
	// 	haptic_set(HAPTIC_RIGHT, HAPTIC_DISABLED, 0.);
	// 	k_sleep(K_SECONDS(1));
	// 	printk("Done.");
	// 	haptic_set(HAPTIC_LEFT, HAPTIC_ENABLED, 0.25f);
	// 	haptic_set(HAPTIC_RIGHT, HAPTIC_ENABLED, 0.8f);
	// 	k_sleep(K_SECONDS(1));
	// 	printk("Done.");
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

		// Perform frequency calculations
		frequency = predict_freq(&uncertainty);
		if (DROP_UNCERTAIN_DATA && uncertainty > UNCERTAINTY_DISCARD)
			continue;
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
		haptic_set_both(HAPTIC_ENABLED, error, HAPTIC_ENABLED, error);

		// Latency testing code
		if (DO_LATENCY_TESTING) {
			nrf_gpio_pin_clear(DEBUG_LATENCY_PIN);
			k_sleep(K_SECONDS(1));
		}
		
		printk("Predicted note: %s,\t\tfrequency: %f,\t\terror: %f,\t\tuncertainty: %f\n", note, frequency, error, uncertainty * 100.f);
	}

}