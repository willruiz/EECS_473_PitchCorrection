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

// Main variables
float frequency;
const char *note;
float error;

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
		frequency = predict_freq();

		// Find matching note and error
		note = find_closest_note(frequency, &error);

		// TODO set motor values here based on error
		// Error is a float between 1 and 100.

		printk("Predicted note: %s,\t\frequency: %f\t\t, error: %f", note, frequency, error);
	}

}