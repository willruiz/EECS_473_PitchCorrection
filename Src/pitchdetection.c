#include "pitchdetection.h"

// Audio buffer.
int32_t *buf;

// Frequency calculation buffers. Do math with these.
float nsdf_buf[MAX_TAU - MIN_TAU + 1] = {0};
float audio_bitshifted[BUF_SIZE] = {0};
float audio_squared[BUF_SIZE] = {0};

// Frequency calculation function declarations. Implemented below.
float nsdf(uint16_t tau); // Normalized square difference function. Used by predict_freq
float acr(uint16_t tau);  // Autocorrelation function. Used by NSDF
float m(uint16_t tau);    // M function (see paper). Used by NSDF

// Predicts the frequency of the audio in the audio buffer.
// O(SAMPLE_RATE^2 * (1/MIN_FREQUENCY - 1/MAX_FREQUENCY)^2)
float predict_freq(float *uncertainty) {
	
	// Declarations
    uint16_t tau; //the current iteration of tau, the period in samples
    float curr_max = 0; //the maximum value of 
    float curr_nsdf;
	uint16_t p_zero = MIN_TAU;
	uint16_t n_zero = MAX_TAU;
	uint16_t i;

	// First average the audio data to normalize it
	int32_t avg = 0;
	for (i = 0; i < BUF_SIZE; i++) {
		avg += buf[i] >> 6;
	}
	avg /= BUF_SIZE;

	// Normalize audio data and convert it actual numbers (I2S data is leading by 6 zeros, so we shift it)
	// Also store the squared values of the audio data so we don't repeat multiplications later
	for(i = 0; i < BUF_SIZE; i++) {
		float val = ((buf[i] >> 6) - avg) / 5000. * 0.0479; // Constants found through testing
		audio_bitshifted[i] = val;
		audio_squared[i] = val * val;
		//printk("val: %f\n", buf[i]); // Uncomment this to print raw audio data
	}


	// Loop through every tau delay period and calculate the NSDF for that value.
	// Simultaneously calculate the maximum NSDF value and the crossing-zero taus
    for (tau = MIN_TAU; tau <= MAX_TAU; tau++) {

		// Calculate normalized square difference function for this period
        curr_nsdf = nsdf(tau);

		//printk("%u, %lf\n", tau, curr_nsdf * 1000); // Uncomment just this print statement to graph NSDF(tau)
        
		nsdf_buf[tau-MIN_TAU] = curr_nsdf;
        if (curr_nsdf > curr_max)
            curr_max = curr_nsdf;

		//   p_zero not found     greater than zero   index                    prev less than zero
		if (p_zero == MIN_TAU && curr_nsdf >= 0 && tau-MIN_TAU > 0 && nsdf_buf[tau-MIN_TAU-1] <= 0) {
			p_zero = tau;
		}
		if (p_zero != MIN_TAU && n_zero == MAX_TAU && curr_nsdf <= 0 && tau-MIN_TAU > 0 && nsdf_buf[tau-MIN_TAU-1] >= 0) {
			n_zero = tau;
		}
    }

    //float threshold = curr_max * K; // Threshold from paper. We currently don't use this though.

	//printk("p_zero: %u, n_zero: %u\n", p_zero, n_zero); // Debug zero-crossing calculations

	// If the data never crosses 0, it is bad and we should quit.
	if (p_zero == MIN_TAU) {
		// Set the uncertainty to one for bad data.
		*uncertainty = 1;
		return 0;
	}
	
    // Find maximum value within the range of crossing-zero taus. This is the most prominent major peak in the graph excluding the beginning
    uint16_t max_tau = 0;
	curr_max = 0;
    for (tau = p_zero; tau < n_zero; tau++) {
        if (curr_max < nsdf_buf[tau-MIN_TAU]) {
			max_tau = tau;
			curr_max = nsdf_buf[tau-MIN_TAU];
		}
    }

	// Set the uncertainty
	*uncertainty = 1 - curr_max;

	// Disregard any calculations done if the data is bad (no peaks found)
    if (max_tau <= 0) {
        //printk("ERROR: No max tau found with threshold %f\n", threshold); // Debug message if there is no peak found.
		
		// Set the uncertainty to one for bad data.
		*uncertainty = 1;
        return 0;
    }

	//printk("Max tau: %u\n", max_tau); // Print calculated maximum tau value

    // Translate sample period to Hz and return

	// These constants transform from the calculated frequency to
	// actual frequency based on a linear fit of the data we collected on 11/3.
	// For 62500 Hz sample rate, 1000-4000 frequency min and max
    return (SAMPLE_RATE / (double)max_tau + 0.2248771853f) / 1.008076169f;
}

// Normalized square difference function, from the paper
// O(SAMPLE_RATE * (1/MIN_FREQUENCY - 1/MAX_FREQUENCY))
float nsdf(uint16_t tau) {
    return 2. * acr(tau) / m(tau);
}

// Autocorrelation function, from the paper
// O(SAMPLE_RATE * (1/MIN_FREQUENCY - 1/MAX_FREQUENCY))
float acr(uint16_t tau) {
    int i;
    float sum = 0;
	for (i = 0; i < BUF_SIZE - 1 - tau; i++) {                                                                                                                                                                                                       
		sum += audio_bitshifted[i] * audio_bitshifted[i + tau];                                                                                                                                                                                                
	}
    return sum;
}

// M function, from the paper
// O(SAMPLE_RATE * (1/MIN_FREQUENCY - 1/MAX_FREQUENCY))
float m(uint16_t tau) {
    int i;                                                                                            
	float sum = 0;
    //calculate each m value                                                                                                                                                                                                                     
	for (i = 0; i < BUF_SIZE - 1 - tau; i++) {
        sum += audio_squared[i] + audio_squared[i + tau];
    }

    return sum;
}    

// Matches the frequency provided with a note and returns the note as a c-string. Also stores the error in error if error is not nullptr
// O(NUM_NOTES)
const char* find_closest_note(float frequency, float *error) {
	for (int i = 1; i < NUM_NOTES; ++i) {
		if (freqs[i] - frequency > 0) {
			float diff_half = (freqs[i] - freqs[i-1]) / 2;
			if (-1*(freqs[i - 1] - frequency ) < (freqs[i] - frequency)) {
				if (error) *error = ((freqs[i - 1] - frequency) / diff_half) * 100.;
				return notes[i-1];
			}
			else {
				if (error) *error = ((freqs[i] - frequency) / diff_half) * 100.;
				return notes[i];
			}
		}
	}
	return "ERROR: NO NOTE FOUND";
}

void set_pitch_buffer(int32_t *buf_) {
	buf = buf_;
}