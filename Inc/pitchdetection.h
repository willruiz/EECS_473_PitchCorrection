#ifndef PITCH_DETECTION_H
#define PITCH_DETECTION_H

#include <zephyr.h>
#include <string.h>
#include "nrfx_i2s.h"
#include "notes.h"
#include "microphone.h"

// Runs the frequency calculation algorithm. Returns the predicted frequency in Hz and stores the uncertainty (1-0, where 0 is most certain and 1 is least certain)
float predict_freq(float *uncertainty);

// Returns a c-string of the closest note to frequency, and stores the percent error (0-100) in error
const char *find_closest_note(float frequency, float *error);

void set_pitch_buffer(int32_t *buf);

#endif