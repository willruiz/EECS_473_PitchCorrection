#include <zephyr.h>

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
	haptic_set(HAPTIC_LEFT, HAPTIC_ENABLED, 0.75f);
	while(1) {}

	// Start microphone
	microphone_start();

	while(1) {

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

		printk("Predicted note: %s,\t\tfrequency: %f,\t\terror: %f,\t\tuncertainty: %f\n", note, frequency, error, uncertainty * 100.f);
	}

}