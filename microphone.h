#ifndef MICROPHONE_H
#define MICROPHONE_H

//TODO: Ellen

// used https://os.mbed.com/users/4180_1/notebook/using-a-microphone-for-audio-input/ for help & inspiration


// I2S microphone here has its own library : https://os.mbed.com/users/p07gbar/code/I2S/log/455d5826751b/I2S.h/

// possibly useful, haven't read entirely: https://www.analog.com/media/en/dsp-documentation/embedded-media-processing/embedded-media-processing-chapter5.pdf

// mircophone interface
class Microphone {
  enum Note {A, As, B, C, Cs, D, Ds, E, F, Fs, G, Gs}; // 's' indicates sharp
  public:
  Note curr_note;
  float perc_error;
  
  //constructor
  // initialize the pin as an (analog) input
  microphone(PinName pin);
  
  private:
  // read in the current value of the microphone, use in interrrupts
  float microphone_read();
  
  // tuning, error checking, note identifying algorithm
  // asign curr_note, perc_error for output to haptic motors and/or screen
  void convert();
  
}


#endif
