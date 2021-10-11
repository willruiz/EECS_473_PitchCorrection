//Uno, Nano, Mini: PWM pins: 3, 5, 6, 9, 10, 11
#define OFF 0
#define PIN_NINE 9

/*
 DOCs:
 Low Vibration: 
 Vibration period = 65ms

 High Vibration:
 
 */

class HIGH_MOTOR {
  private:
    int pulse = 65;
    int offpulse = 15;
    int str = 160;
    

  public:
    void setup_high() {
      Serial.begin(9600);
      pinMode(PIN_NINE, OUTPUT);
    }
    void setup_high(int pulse_in, int off_pulse_in, int str_in) {
      Serial.begin(9600);
      pinMode(PIN_NINE, OUTPUT);
      pulse = pulse_in;
      offpulse = off_pulse_in;
      str = str_in;
    }
    void testpulse() {
          analogWrite(PIN_NINE, str);
          delay(pulse);

          analogWrite(PIN_NINE, OFF);
          delay(offpulse);
    }
};

class LOW_MOTOR {
  private:
    int pulse = 65;
    int str = 255;

  public:
    void setup_low() {
      Serial.begin(9600);
      pinMode(PIN_NINE, OUTPUT);
    }
    void setup_low(int pulse_in, int str_in) {
      Serial.begin(9600);
      pinMode(PIN_NINE, OUTPUT);
      pulse = pulse_in;
      str = str_in;
    }
    void testpulse() {
          analogWrite(PIN_NINE, str);
          delay(pulse);

          analogWrite(PIN_NINE, OFF);
          delay(pulse);
    }
};

LOW_MOTOR lm;
HIGH_MOTOR hm;
void setup() {
  pinMode(13, OUTPUT);
  lm.setup_low(65, 255);
  hm.setup_high(65, 15, 170);
}

// the loop function runs over and over again forever
int i;
void loop() {
   digitalWrite(13, HIGH);
   for (i = 0; i < 30; ++i) {
      lm.testpulse();
   }
   digitalWrite(13, LOW);
   for (i = 0; i < 30; ++i) {
      hm.testpulse();
   }
}
