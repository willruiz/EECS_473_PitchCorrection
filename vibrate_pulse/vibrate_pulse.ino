//Uno, Nano, Mini: PWM pins: 3, 5, 6, 9, 10, 11
#define OFF 0
int pin_nine = 9;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  pinMode(pin_nine, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  //analogWrite(pin_nine, 255); // min threshold: 65
}

// the loop function runs over and over again forever
int i = 0;
int pulse = 65;
int off_pulse = 15;
int strength = 255;
int sharp_str = 170;
int stime = 50;

void loop() {

//  // SHARP
//  for (i = 0; i < stime; ++i) {
//      analogWrite(pin_nine, sharp_str);
//      delay(pulse);
//
//      analogWrite(pin_nine, OFF);
//      delay(off_pulse);
//    
//  }
//  // FLAT
//  for (i = 0; i < stime; ++i) {
//      analogWrite(pin_nine, strength);
//      delay(pulse);
//
//      analogWrite(pin_nine, OFF);
//      delay(pulse);
//    
//  }
  
//  for (i = 0; i < 255; ++i) {
//    Serial.println(i);
//    delay(10);
//    analogWrite(pin_nine, i);
//    //pinMode(LED_BUILTIN, OUTPUT);
//  }
//  for (i = 255; i > 0; --i) {
//    Serial.println(i);
//    delay(10);
//    analogWrite(pin_nine, i);
//  }
}
