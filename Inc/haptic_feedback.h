#ifndef HAPTIC_H
#define HAPTIC_H

#include <stdint.h>

#define HAPTIC_LEFT 0
#define HAPTIC_RIGHT 1

#define HAPTIC_DISABLED 0
#define HAPTIC_ENABLED 1

//Haptic values:
//  Enable: if the motor is enabled
//  Strength: the relative vibrational strength

// Initializes haptic interface with default parameters.
void haptic_init(uint8_t left_pin, uint8_t right_pin);

// Sets the haptic values for a given side (HAPTIC_LEFT or HAPTIC_RIGHT).
void haptic_set(uint8_t haptic, uint8_t enabled, float strength);

// Sets the haptic values for both sides.
void haptic_set_both(uint8_t left_en_in, uint8_t right_en_in, float left_str_in, float right_str_in);

// Sets the haptic values to the defaults.
void haptic_default_on();
    
// Sets the strength of both motors.
void haptic_set_both_strength(float strength);

// Enables or disables both motors.
void haptic_set_both_enable(uint8_t enable);

// Enables or disables a motor.
void haptic_enable(uint8_t haptic, uint8_t enable);

// Sets the strength of a motor.
void haptic_set_strength(uint8_t haptic, float strength);

#endif