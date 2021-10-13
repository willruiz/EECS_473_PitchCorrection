#include <zephyr.h>
#include <string.h>
#include <hal/nrf_pwm.h>
#include <hal/nrf_gpio.h>

#include "nrfx_i2s.h"

//#error Remember to define these pins before you run! // This assume you only have these 3 pins for I2S.
#define I2S_WS_PIN 4
#define I2S_SD_PIN 5
#define I2S_SCK_PIN 2
// #define SCK_PIN 2      //Serial clock for i2s
// #define LRCK_PIN 4     //Word select clock
// #define SDIN_PIN 5    //Data Input

#define I2S_DATA_BLOCK_WORDS 512 //How many numbers do we want. time reorded = DATA_BLOCK_WORDS / freq

static uint32_t m_buffer_rx32u[I2S_DATA_BLOCK_WORDS];
static int32_t tmp[I2S_DATA_BLOCK_WORDS];
static nrfx_i2s_buffers_t initial_buffers;
static bool data_ready_flag = false;

ISR_DIRECT_DECLARE(i2s_isr_handler)
{
  //printk("isr fired...\n");
	data_ready_flag = false;
	nrfx_i2s_irq_handler();
	ISR_DIRECT_PM(); /* PM done after servicing interrupt for best latency
			  */
	return 1;		 /* We should check if scheduling decision should be made */
}

static void data_handler(nrfx_i2s_buffers_t const *p_released, uint32_t status)
{
  //printk("handling new data...\n");
	if (NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED == status)
	{
		nrfx_err_t err = nrfx_i2s_next_buffers_set(&initial_buffers);
		if (err != NRFX_SUCCESS)
		{
			printk("Error!, continuing running as if nothing happened, but you should probably investigate.\n");
		}
	}
	if (p_released)
	{
		if (p_released->p_rx_buffer != NULL)
		{
			data_ready_flag = true; //This is used in print_sound()
		}
	}
}

void print_sound()
{
	while (!data_ready_flag)
	{
		k_sleep(K_MSEC(1));
    //printk("waiting...\n");
		//Wait for data. Since we do not want I2S_DATA_BLOCK_WORDS amount of prints inside the interrupt.
	}
  //printk("Printing sound...\n");
	//nrfx_i2s_stop();
	data_ready_flag = false;

	/** Print the raw data from the I2S microphone */
	
	for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
	{
    //shift by 18 from mic datasheet
		printk("%d, \n", m_buffer_rx32u[i] >> 6); //The audio is automatically saved in m_buffer_rx32u by the interrupt
	}
	printk("\n\n");
	

	/**  Format the data from Adafruit I2S MEMS Microphone Breakout, then print it	*/

	// int64_t sum = 0;
	// int64_t words = I2S_DATA_BLOCK_WORDS;

	// for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
	// {
	// 	memcpy(tmp + i, m_buffer_rx32u + i, sizeof(uint32_t));
	// 	tmp[i] >>= 8;
	// 	sum += tmp[i];
	// }

	// int64_t mean = sum / words;
	// for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
	// {
	// 	tmp[i] -= mean;
	// 	printk("%d, ", tmp[i]);
	// 	k_sleep(K_MSEC(16));
	// }
	// printk("\n\n");
	// /**  End of formatted data print*/
}

nrfx_err_t get_sound_init()
{
	IRQ_DIRECT_CONNECT(I2S_IRQn, 0, i2s_isr_handler, 0);
	memset(&m_buffer_rx32u, 0, sizeof(m_buffer_rx32u));
	initial_buffers.p_rx_buffer = m_buffer_rx32u;

	//YOu should probably change this config to fit your I2S microphone and audio preferences.
	nrfx_i2s_config_t config =
		NRFX_I2S_DEFAULT_CONFIG(I2S_SCK_PIN, I2S_WS_PIN,
								NRFX_I2S_PIN_NOT_USED,
								NRFX_I2S_PIN_NOT_USED, I2S_SD_PIN);

	config.mode = NRF_I2S_MODE_SLAVE;			//Microphone requirement
	config.ratio = NRF_I2S_RATIO_64X;			//Microphone requirement
	config.sample_width = NRF_I2S_SWIDTH_24BIT; //Microphone requirement
	config.mck_setup = NRF_I2S_MCK_32MDIV31;	//Preference     freq = (MCKfreq/ratio) =16.129 KHz.
	config.channels = NRF_I2S_CHANNELS_LEFT;	//Preference
  	config.alignment = NRF_I2S_ALIGN_LEFT;

	nrfx_err_t err_code = nrfx_i2s_init(&config, data_handler);
	if (err_code != NRFX_SUCCESS)
	{
		printk("I2S init error\n");
		return err_code;
	}

  
	err_code = nrfx_i2s_start(&initial_buffers, I2S_DATA_BLOCK_WORDS, 0); //start recording
	if (err_code != NRFX_SUCCESS)
	{
		printk("I2S start error\n");
		return err_code;
	}
	//nrfx_i2s_stop() //stop recording

	k_sleep(K_SECONDS(2));
	return err_code;
}

void main()
{

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
	nrfx_err_t err;

	printk("Min:-65538,Max:65538\n");

	err = get_sound_init();
	if (err != NRFX_SUCCESS)
	{
		return;
	}
	while (1)
	{
		//k_sleep(K_MSEC(1000));
		print_sound();
	}
}

//OLD CODE NOT USED ANYMORE

// /*
//  * Copyright (c) 2012-2014 Wind River Systems, Inc.
//  *
//  * SPDX-License-Identifier: Apache-2.0
//  */

// #include <zephyr.h>
// #include <sys/printk.h>
// #include <nrf.h>
// #include <hal/nrf_gpio.h>
// #include <hal/nrf_pwm.h>
// #include <nrfx_i2s.h>
// #include <stdbool.h>
// #include <stdint.h>


// //Audio constants
// #define WINDOW_SIZE 0.1 //seconds
// #define SAMPLE_RATE 16129.0328125 //Hz
// #define BUF_SIZE (int)(WINDOW_SIZE*SAMPLE_RATE)

// static uint32_t audio_buf[BUF_SIZE];
// static nrfx_i2s_buffers_t initial_buffers;
// static bool data_ready_flag = false;

// int i=0;

// ISR_DIRECT_DECLARE(i2s_isr_handler)
// {
// 	data_ready_flag = false;
// 	nrfx_i2s_irq_handler();
// 	ISR_DIRECT_PM(); /* PM done after servicing interrupt for best latency
// 			  */
// 	return 1;		 /* We should check if scheduling decision should be made */
// }

// static void data_handler(nrfx_i2s_buffers_t const *p_released, uint32_t status)
// {
// 	if (NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED == status)
// 	{
// 		nrfx_err_t err = nrfx_i2s_next_buffers_set(&initial_buffers);
// 		if (err != NRFX_SUCCESS)
// 		{
// 			printk("Error!, continuing running as if nothing happened, but you should probably investigate.\n");
// 		}
// 	}
// 	if (p_released)
// 	{
// 		if (p_released->p_rx_buffer != NULL)
// 		{
// 			data_ready_flag = true; //This is used in print_sound()
// 		}
// 	}
// }

// //irq
// void i2s_irq_handler(void);

// #define SCK_PIN 2      //Serial clock for i2s
// #define LRCK_PIN 4     //Word select clock
// #define SDIN_PIN 5    //Data Input
// #define NC 0xFFFFFFFF   //Not connected

// void main(void)
// {
// 	printk("Hello World! %s\n", CONFIG_BOARD);
//   nrf_gpio_cfg_output(27);
//   nrf_gpio_cfg_output(30);

//   //pwm
//   uint32_t out_pins[] = {28, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};
//   static uint16_t pwm_seq[2] = {0x8003, 0x0004};
//   nrf_pwm_sequence_t const seq = 
//   {
//       .values.p_common = pwm_seq,
//       .length          = sizeof(pwm_seq)/sizeof(uint16_t),
//       .repeats         = 0,
//       .end_delay       = 0
//   };

//   nrf_gpio_cfg_output(28);
//   nrf_pwm_pins_set(NRF_PWM0, out_pins);
//   nrf_pwm_enable(NRF_PWM0);
//   //2.048MHz
//   nrf_pwm_configure(NRF_PWM0, NRF_PWM_CLK_16MHz, NRF_PWM_MODE_UP, 0x7);
//   nrf_pwm_loop_set(NRF_PWM0, 0);
//   nrf_pwm_decoder_set(NRF_PWM0, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
//   nrf_pwm_sequence_set(NRF_PWM0, 0, &seq);
//   NRF_PWM0->TASKS_SEQSTART[0] = 1;

//   uint32_t out_pins2[] = {NRF_PWM_PIN_NOT_CONNECTED, 29, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};
//   static uint16_t pwm_seq2[2] = {0x8090, 0x00C0};
//   nrf_pwm_sequence_t const seq2 = 
//   {
//       .values.p_common = pwm_seq2,
//       .length          = sizeof(pwm_seq2)/sizeof(uint16_t),
//       .repeats         = 0,
//       .end_delay       = 0
//   };
//   nrf_gpio_cfg_output(28);
//   nrf_pwm_pins_set(NRF_PWM1, out_pins2);
//   nrf_pwm_enable(NRF_PWM1);
//   //2.048MHz / 4
//   nrf_pwm_configure(NRF_PWM1, NRF_PWM_CLK_16MHz, NRF_PWM_MODE_UP, 0x1C0);
//   nrf_pwm_loop_set(NRF_PWM1, 0);
//   nrf_pwm_decoder_set(NRF_PWM1, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
//   nrf_pwm_sequence_set(NRF_PWM1, 0, &seq2);
//   NRF_PWM1->TASKS_SEQSTART[0] = 1;


//   //i2s
//   IRQ_DIRECT_CONNECT(I2S0_IRQn, 0, i2s_irq_handler, 0);
//   memset(&audio_buf, 0x00, sizeof(audio_buf));
//   initial_buffers.p_rx_buffer = audio_buf;
//   initial_buffers.p_tx_buffer = NULL;

//   nrfx_i2s_config_t config =
//       NRFX_I2S_DEFAULT_CONFIG(SCK_PIN, LRCK_PIN, NRFX_I2S_PIN_NOT_USED, NRFX_I2S_PIN_NOT_USED, SDIN_PIN);
  
//   config.mode = NRF_I2S_MODE_SLAVE;
//   config.ratio = NRF_I2S_RATIO_64X;
//   config.sample_width = NRF_I2S_SWIDTH_24BIT;
//   config.mck_setup = NRF_I2S_MCK_32MDIV31;
//   config.channels = NRF_I2S_CHANNELS_LEFT;

//   nrfx_err_t err_code = nrfx_i2s_init(&config, data_handler);

//   // bool i2s_config_status;

// 	// nrf_i2s_pins_set(NRF_I2S, SCK_PIN, LRCK_PIN, NC, NC, SDIN_PIN);
//   // i2s_config_status = nrf_i2s_configure(NRF_I2S,
//   //                   NRF_I2S_MODE_SLAVE,
//   //                   NRF_I2S_FORMAT_I2S,
//   //                   NRF_I2S_ALIGN_LEFT,
//   //                   NRF_I2S_SWIDTH_24BIT,
//   //                   NRF_I2S_CHANNELS_LEFT, //?
//   //                   NRF_I2S_MCK_32MDIV31, //1.0322581MHz, 1.024Mhz clock gives 16kHz sampling
//   //                   NRF_I2S_RATIO_64X //?
//   // );

//   // nrf_gpio_pin_clear(27);
//   // nrf_gpio_pin_clear(30);
//   // i2s_config_status ? nrf_gpio_pin_set(27) : nrf_gpio_pin_set(30);

//   // nrf_i2s_transfer_set(NRF_I2S, (uint16_t)BUF_SIZE, audio_buf, NULL);

//   // nrf_i2s_enable(NRF_I2S);

//   //interrupt stuff
//   //nrf_i2s_int_enable(NRF_I2S, NRF_I2S_INT_RXPTRUPD_MASK);

//   //printk("Before fault\n");

//   //NVIC_SetVector(I2S_IRQn, (uint32_t)i2s_irq_handler);
  

//   //printk("After fault\n");
//   //while(1) {}

//   //NRFX_IRQ_PRIORITY_SET(I2S_IRQn, NRFX_I2S_DEFAULT_CONFIG_IRQ_PRIORITY);

//   //NRFX_IRQ_ENABLE(I2S_IRQn);

// 	while(1) {
// 		printk("Data: %u\n", audio_buf[0]);
// 	}

// }

// void i2s_irq_handler(void) {

//   // if (nrf_i2s_event_check(NRF_I2S, NRF_I2S_EVENT_RXPTRUPD))
//   // {
//   //   //clear RXPTRUPD event
//   //   nrf_i2s_event_clear(NRF_I2S, NRF_I2S_EVENT_RXPTRUPD);
    
//   //   printk("Interrupt fired!\n");

//   // }

// }
