#include <stdint.h>
#include <zephyr.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_pwm.h>

#include "haptic_feedback.h"

#define HAPTIC_PWM_CLOCK NRF_PWM_CLK_16MHz
#define HAPTIC_PWM_PERIOD 16000 //1ms


// Pins
uint8_t left_pin;
uint8_t right_pin;

// The "strength" of the vibration. Maps directly to PWM duty cycle under the hood.
float left_strength;
float right_strength;

// If the motors are enabled at all.
uint8_t left_en;
uint8_t right_en;

// PWM sequences for left and right.
uint16_t pwm_seq[4] = {0 /* Left */, 0 /* Right */, 0 /* Unused */, 0 /* Unused */};

// CRITICAL HARDWARE INTERFACING FUNCTION- SETS PWM DUTY CYCLES
// DO NOT CALL THIS FROM APPLICATION CODE
void set_vibration() { 

    // Left motor
    // Check boundary conditions
    left_strength = (left_strength < 0.f)? 0.f : (left_strength > 1.f)? 1.f : left_strength;
    // Write to sequence
    pwm_seq[0] = (left_en == HAPTIC_ENABLED)? (uint16_t)((float)(HAPTIC_PWM_PERIOD * left_strength)) : 0;

    printk("left: %u\n", pwm_seq[0]);

    // Right motor
    // Check boundary conditions
    right_strength = (right_strength < 0.f)? 0.f : (right_strength > 1.f)? 1.f : right_strength;
    // Write to sequence
    pwm_seq[1] = (right_en == HAPTIC_ENABLED)? (uint16_t)((float)(HAPTIC_PWM_PERIOD * right_strength)) : 0;

    if (NRF_PWM2->EVENTS_SEQEND[0]) {
        NRF_PWM2->TASKS_SEQSTART[0] = 1;
    }

}

// Initializes haptic interface with default parameters.
void haptic_init(uint8_t left_pin_, uint8_t right_pin_) {
    // Set pins
    left_pin = left_pin_;
    right_pin = right_pin_;

    // PWM configuration
    pwm_seq[0] = 0; // Left PWM
    pwm_seq[1] = 0; // Right PWM
    pwm_seq[2] = 0; // Unused
    pwm_seq[3] = 0; // Unused
    NRF_PWM2->PSEL.OUT[0] = (left_pin << PWM_PSEL_OUT_PIN_Pos) | (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
    NRF_PWM2->PSEL.OUT[1] = (right_pin << PWM_PSEL_OUT_PIN_Pos) | (PWM_PSEL_OUT_CONNECT_Connected << PWM_PSEL_OUT_CONNECT_Pos);
    NRF_PWM2->ENABLE      = (PWM_ENABLE_ENABLE_Enabled << PWM_ENABLE_ENABLE_Pos);
    NRF_PWM2->MODE        = (PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos);
    NRF_PWM2->PRESCALER   = (PWM_PRESCALER_PRESCALER_DIV_16 << PWM_PRESCALER_PRESCALER_Pos);
    NRF_PWM2->COUNTERTOP  = (HAPTIC_PWM_PERIOD << PWM_COUNTERTOP_COUNTERTOP_Pos);
    NRF_PWM2->LOOP        = (PWM_LOOP_CNT_Disabled << PWM_LOOP_CNT_Pos);
    NRF_PWM2->DECODER   = (PWM_DECODER_LOAD_Individual << PWM_DECODER_LOAD_Pos) | 
                      (PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos);
    NRF_PWM2->SEQ[0].PTR  = ((uint32_t)(pwm_seq) << PWM_SEQ_PTR_PTR_Pos);
    NRF_PWM2->SEQ[0].CNT  = ((sizeof(pwm_seq) / sizeof(uint16_t)) <<
                                                 PWM_SEQ_CNT_CNT_Pos);
    NRF_PWM2->SEQ[0].REFRESH  = 0;
    NRF_PWM2->SEQ[0].ENDDELAY = 0;
    NRF_PWM2->TASKS_SEQSTART[0] = 1;

    left_en = 0;
    right_en = 0;
    left_strength = 0;
    right_strength = 0;
    set_vibration();
}

// Sets the haptic values for a given side (HAPTIC_LEFT or HAPTIC_RIGHT).
void haptic_set(uint8_t haptic, uint8_t enabled, float strength) {
    if (haptic == HAPTIC_LEFT) {
        left_en = enabled;
        left_strength = strength;
    } else {
        right_en = enabled;
        right_strength = strength;
    }
    set_vibration();
}

// Sets the haptic values for both sides.
void haptic_set_both(uint8_t left_en_in, uint8_t right_en_in, float left_str_in, float right_str_in) { // full custom constructor
    left_en = left_en_in;
    right_en = right_en_in;
    left_strength = left_str_in;
    right_strength = right_str_in;
    set_vibration();
}

// Sets the haptic values to the defaults.
void haptic_default_on() {
    left_en = 1;
    right_en = 1;
    left_strength = 0.2; // Arbitrary value, maybe change
    right_strength = 0.2;
    set_vibration();
}

// Sets the strength of both motors.
void haptic_set_both_strength(float strength) {
    left_strength = strength;
    right_strength = strength;
    set_vibration();
}

// Enables or disables both motors.
void haptic_set_both_enable(uint8_t enable) {
    left_en = enable;
    right_en = enable;
    set_vibration();
}

// Enables or disables a motor.
void haptic_enable(uint8_t haptic, uint8_t enable) {
    if (haptic == HAPTIC_LEFT) {
        left_en = enable;
    } else {
        right_en = enable;
    }
    set_vibration();
}

// Sets the strength of a motor.
void haptic_set_strength(uint8_t haptic, float strength) {
    if (haptic == HAPTIC_LEFT) {
        left_strength = strength;
    } else {
        right_strength = strength;
    }
    set_vibration();
}