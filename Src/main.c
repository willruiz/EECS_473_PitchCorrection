#include <zephyr.h>

#include "microphone.h"
#include "pitchdetection.h"

// Selected pins
#define I2S_LRCK_PIN 4
#define I2S_DIN_PIN 5
#define I2S_BCLK_PIN 2
#define I2S_BCLK_PWM_PIN 28
#define I2S_LRCK_PWM_PIN 29
// TODO add motor pins here

// The uncertainty at which we no longer trust the data
#define UNCERTAINTY_DISCARD 0.5f
#define DROP_UNCERTAIN_DATA 1 // Change this to zero to turn dropping off

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

	// Start microphone
	microphone_start();

	while(1) {

		// Perform frequency calculations
		frequency = predict_freq(&uncertainty);
#if DROP_UNCERTAIN_DATA != 0
		if (uncertainty > UNCERTAINTY_DISCARD)
			continue;
#endif
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

		// TODO set motor values here based on error
		// Error is a float between 1 and 100.

		printk("Predicted note: %s,\t\tfrequency: %f,\t\terror: %f,\t\tuncertainty: %f\n", note, frequency, error, uncertainty * 100.f);
	}

}