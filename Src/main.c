/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <nrf.h>
#include <hal/nrf_gpio.h>
#include <hal/nrf_pwm.h>
#include <nrfx_i2s.h>
#include <stdbool.h>
#include <stdint.h>


//Audio constants
#define WINDOW_SIZE 0.1 //seconds
#define SAMPLE_RATE 16129.0328125 //Hz
#define BUF_SIZE (int)(WINDOW_SIZE*SAMPLE_RATE)

uint32_t audio_buf[BUF_SIZE];
int i=0;




//irq
void i2s_irq_handler(void);

#define SCK_PIN 2      //Serial clock for i2s
#define LRCK_PIN 4     //Word select clock
#define SDIN_PIN 5    //Data Input
#define NC 0xFFFFFFFF   //Not connected

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
  nrf_gpio_cfg_output(27);
  nrf_gpio_cfg_output(30);

  //pwm
  uint32_t out_pins[] = {28, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};
  static uint16_t pwm_seq[2] = {0x8003, 0x0004};
  nrf_pwm_sequence_t const seq = 
  {
      .values.p_common = pwm_seq,
      .length          = sizeof(pwm_seq)/sizeof(uint16_t),
      .repeats         = 0,
      .end_delay       = 0
  };

  nrf_gpio_cfg_output(28);
  nrf_pwm_pins_set(NRF_PWM0, out_pins);
  nrf_pwm_enable(NRF_PWM0);
  //2.048MHz
  nrf_pwm_configure(NRF_PWM0, NRF_PWM_CLK_16MHz, NRF_PWM_MODE_UP, 0x7);
  nrf_pwm_loop_set(NRF_PWM0, 0);
  nrf_pwm_decoder_set(NRF_PWM0, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
  nrf_pwm_sequence_set(NRF_PWM0, 0, &seq);
  NRF_PWM0->TASKS_SEQSTART[0] = 1;

  uint32_t out_pins2[] = {NRF_PWM_PIN_NOT_CONNECTED, 29, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};
  static uint16_t pwm_seq2[2] = {0x8090, 0x00C0};
  nrf_pwm_sequence_t const seq2 = 
  {
      .values.p_common = pwm_seq2,
      .length          = sizeof(pwm_seq2)/sizeof(uint16_t),
      .repeats         = 0,
      .end_delay       = 0
  };
  nrf_gpio_cfg_output(28);
  nrf_pwm_pins_set(NRF_PWM1, out_pins2);
  nrf_pwm_enable(NRF_PWM1);
  //2.048MHz / 4
  nrf_pwm_configure(NRF_PWM1, NRF_PWM_CLK_16MHz, NRF_PWM_MODE_UP, 0x1C0);
  nrf_pwm_loop_set(NRF_PWM1, 0);
  nrf_pwm_decoder_set(NRF_PWM1, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
  nrf_pwm_sequence_set(NRF_PWM1, 0, &seq2);
  NRF_PWM1->TASKS_SEQSTART[0] = 1;

  bool i2s_config_status;

	nrf_i2s_pins_set(NRF_I2S, SCK_PIN, LRCK_PIN, NC, NC, SDIN_PIN);
  i2s_config_status = nrf_i2s_configure(NRF_I2S,
                    NRF_I2S_MODE_SLAVE,
                    NRF_I2S_FORMAT_I2S,
                    NRF_I2S_ALIGN_LEFT,
                    NRF_I2S_SWIDTH_24BIT,
                    NRF_I2S_CHANNELS_LEFT, //?
                    NRF_I2S_MCK_32MDIV31, //1.0322581MHz, 1.024Mhz clock gives 16kHz sampling
                    NRF_I2S_RATIO_64X //?
  );

  nrf_gpio_pin_clear(27);
  nrf_gpio_pin_clear(30);
  i2s_config_status ? nrf_gpio_pin_set(27) : nrf_gpio_pin_set(30);

  nrf_i2s_transfer_set(NRF_I2S, (uint16_t)BUF_SIZE, audio_buf, NULL);

  nrf_i2s_enable(NRF_I2S);

  //interrupt stuff
  nrf_i2s_int_enable(NRF_I2S, NRF_I2S_INT_RXPTRUPD_MASK);

  __NVIC_SetVector(I2S_IRQn, (uint32_t)i2s_irq_handler);

  printk("Made it to while loop\n");
  while(1) {}

  NRFX_IRQ_PRIORITY_SET(I2S_IRQn, NRFX_I2S_DEFAULT_CONFIG_IRQ_PRIORITY);

  NRFX_IRQ_ENABLE(I2S_IRQn);

	
	while(1) {
		printk("Data: %u\n", audio_buf[0]);
	}

}

void i2s_irq_handler(void) {

  if (nrf_i2s_event_check(NRF_I2S, NRF_I2S_EVENT_RXPTRUPD))
  {
    //clear RXPTRUPD event
    nrf_i2s_event_clear(NRF_I2S, NRF_I2S_EVENT_RXPTRUPD);
    
    printk("Interrupt fired!\n");

  }

}