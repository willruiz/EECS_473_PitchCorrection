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

//Initializes haptic interface with default parameters.
void Haptic_init();

//Sets the haptic values for a given side (HAPTIC_LEFT or HAPTIC_RIGHT).
void Haptic_set(uint8_t haptic, uint8_t enabled, uint8_t strength);

//Sets the haptic values for both sides.
void Haptic_set_both(uint8_t left_en_in, uint8_t right_en_in, int left_str_in, int right_str_in);

//Sets the haptic values to the defaults.
void Haptic_default_on();
    
//Sets the strength of both motors.
void Haptic_set_both_strength(uint8_t strength);

//Enables or disables both motors.
void Haptic_set_both_enable(uint8_t enable);

//Enables a motor.
void Haptic_enable(uint8_t haptic, uint8_t enabled);

//Sets the strength of a motor.
void Haptic_set_strength(uint8_t haptic, uint8_t strength);

//Tests the range of the motors. Only use this to debug.
void Haptic_test_both_range(uint8_t max);


#endif