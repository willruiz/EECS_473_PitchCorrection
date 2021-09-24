#ifndef HAPTIC_H
#define HAPTIC_H

class Haptic_interface {
    private:

    //bool enabled;
    int left_strength; // controlled by the amount of current being sent
    int right_strength;
    bool left_en;
    bool right_en;
    int MAX_STRENGTH = 100; // strength is choosen to be 0-100
    int MIN_STRENGTH = 0;
    
    public:

    Haptic_interface() { // default constructor
        left_en = false;
        right_en = false;
        left_strength = 0;
        right_strength = 0;
    }

    Haptic_interface(int left_str_in, int right_str_in) { // strength constructor
        left_en = true;
        right_en = true;
        left_strength = left_str_in;
        right_strength = right_str_in;
    }

    Haptic_interface(bool left_en_in, bool right_en_in, 
                        int left_str_in, int right_str_in) { // full custom constructor
        left_en = left_en_in;
        right_en = right_en_in;
        left_strength = left_str_in;
        right_strength = right_str_in;
    }

    void set_vibration() { // CRITICAL HARDWARE INTERFACING FUNCTION
        // functions that uses private member variables
        // and writePins() to appropriate addresses
    }

    void default_on() { // default ON settings (not a constructor)
        left_en = true;
        right_en = true;
        left_strength = 20; // arbitrarily set to 20
        right_strength = 20;
        set_vibration();
    }

    void mini_delay(){
        for (int j = 0; j < 100; ++j) { 
            int k = j; // purely delay functionality
        }
    }
    
    void set_both_strength(int power) {
        strength = power;
        set_vibration();
    }

    void set_enable() {
        left_en = true;
        right_en = true;
        set_vibration();
    }

    void set_disable() {
        left_en = false;
        right_en = false;
        set_vibration();
    }

    void left_enable() {
        left_en = true;
        set_vibration();
    }

    void left_disable() {
        left_en = false;
        set_vibration();
    }

    void right_enable() {
        right_en = true;
        set_vibration();
    }
    
    void right_disable() {
        right_en = false;
        set_vibration();
    }

    void left_set_str(int power) {
        left_strength = power;
        set_vibration();
    }

    void right_set_str(int power) {
        right_strength = power;
        set_vibration();
    }

    void test_both_range(int max) {
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


#endif