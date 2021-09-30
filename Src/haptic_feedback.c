#include "haptic_feedback.h"

//This file needs a bit of fixing up

//bool enabled;
uint8_t left_strength; // controlled by the amount of current being sent
uint8_t right_strength;

uint8_t left_en; //was bool
uint8_t right_en; //was bool

#define MAX_STRENGTH 100 // strength is choosen to be 0-100
#define MIN_STRENGTH 0

void Haptic_set_vibration() { // CRITICAL HARDWARE INTERFACING FUNCTION
    // functions that uses variables
    // and writePins() to appropriate addresses
}

void Haptic_init() { // default constructor
    left_en = 0;
    right_en = 0;
    left_strength = 0;
    right_strength = 0;
}

void Haptic_set_both(uint8_t left_en_in, uint8_t right_en_in, int left_str_in, int right_str_in) { // full custom constructor
    left_en = left_en_in;
    right_en = right_en_in;
    left_strength = left_str_in;
    right_strength = right_str_in;
}

void Haptic_default_on() { // default ON settings (not a constructor)
    left_en = 1;
    right_en = 1;
    left_strength = 20; // arbitrarily set to 20
    right_strength = 20;
    Haptic_set_vibration();
}
    
void Haptic_set_both_strength(uint8_t power) {
    left_strength = power;
    right_strength = power;
    Haptic_set_vibration();
}

void Haptic_set(uint8_t haptic, uint8_t enabled, uint8_t strength) {
    if (haptic == HAPTIC_LEFT)
    left_en = 1;
    right_en = 1;
    Haptoc_set_vibration();
}

void Haptic_left_enable() {
    left_en = 1;
    Haptic_set_vibration();
}

void Haptic_left_disable() {
    left_en = 0;
    Haptic_set_vibration();
}

void Haptic_right_enable() {
    right_en = 1;
    Haptic_set_vibration();
}

void Haptic_right_disable() {
    right_en = 0;
    Haptic_set_vibration();
}

void Haptic_left_set_str(int power) {
    left_strength = power;
    Haptic_set_vibration();
}

void Haptic_right_set_str(int power) {
    right_strength = power;
    Haptic_set_vibration();
}

void Haptic_test_both_range(int max) {
    if (max > MAX_STRENGTH) { 
        max = MAX_STRENGTH;
    }
    else if (max < MIN_STRENGTH) {
        max = MIN_STRENGTH;
    }
    set_enable();
    for (int i = 0; i < max; ++i) {
        set_both_strength(i);
        mini_delay();
    }
    set_disable();
}