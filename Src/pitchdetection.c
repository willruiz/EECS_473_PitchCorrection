#include <zephyr.h>
#include <string.h>
#include <hal/nrf_pwm.h>
#include <hal/nrf_gpio.h>

#include "nrfx_i2s.h"

#define MAX_FREQUENCY 10000 //hz
#define MIN_FREQUENCY 30 //hz
                                                                                                                                                                                                                   
#define SAMPLE_RATE 35370 //samples / second, Checked with oscope on mclk                                                                                                                                                                                                  
#define MIN_TAU SAMPLE_RATE / MAX_FREQUENCY                                                                                                                                                                                            
#define MAX_TAU SAMPLE_RATE / MIN_FREQUENCY
#define BUF_SIZE MAX_TAU * 2 //we want at least MAX_TAU samples to math on for a given tau
#define K 0.98

//frequency calculation and audio buffers
double nsdf_buf[MAX_TAU - MIN_TAU + 1];
int32_t audio_buf[BUF_SIZE];

//frequency calculation functions
double predict_freq();
double nsdf(uint16_t tau);
int64_t acr(uint16_t tau);
int64_t m(uint16_t tau);

//Put main() here for testing

//O(MAX_TAU * WINDOW_SIZE * SAMPLE_RATE)
double predict_freq() {

    //iterate over 1 to max tau and calculate NSDF
    //TODO optimize this
    uint16_t tau;
    double curr_max = 0;
    double curr_nsdf;
	uint16_t tau_of_first_positive_sloped_zero_crossing = MIN_TAU;

    for (tau = MIN_TAU; tau <= MAX_TAU; tau++) {
        curr_nsdf = nsdf(tau);
        nsdf_buf[tau-MIN_TAU] = curr_nsdf;
        if (curr_nsdf > curr_max)
            curr_max = curr_nsdf;
		//printk("NSDF for tau %u: %lf\n", tau, curr_nsdf);
		if (tau_of_first_positive_sloped_zero_crossing == 0 && curr_nsdf > 0 && tau-MIN_TAU > 0 && nsdf_buf[tau-MIN_TAU-1] < 0) {
			tau_of_first_positive_sloped_zero_crossing = tau;
		}
    }
    double threshold = curr_max * K;

    //find first local maxima above threshold
    uint16_t max_tau = 0;
    for (tau = tau_of_first_positive_sloped_zero_crossing; tau < MAX_TAU-1; tau++) {
        double curr = nsdf_buf[tau-MIN_TAU];
        double prev = nsdf_buf[tau-1-MIN_TAU];
        double next = nsdf_buf[tau+1-MIN_TAU];
        if (curr > threshold && prev <= curr && next <= curr) {
            max_tau = tau;
			break;
        }
    }

    if (max_tau <= 0) {
        printk("ERROR: No max tau found with threshold %f\n", threshold);
        return 0;
    }

	//printk("Max tau: %u\n", max_tau);

    //translate sample period to Hz and return
    return SAMPLE_RATE / (double)max_tau;
}

//O(O(acr) + O(m))
double nsdf(uint16_t tau) {
    return 2. * acr(tau) / m(tau);
}

//O(window size * sample rate)
int64_t acr(uint16_t tau) {
    int i;
    int64_t sum = 0;
    //calculate each acr value                                                                                                                                                                                                                   
	for (i = 0; i < BUF_SIZE - 1 - tau; i++) {                                                                                                                                                                                                       
		sum += (audio_buf[i] >> 6) * (audio_buf[i + tau] >> 6);                                                                                                                                                                                                
	}

    return sum; //bitshift after math?
}


//O(window size * sample rate)
int64_t m(uint16_t tau) {
    int i;                                                                                            
	int64_t sum = 0;
    //calculate each m value                                                                                                                                                                                                                     
	for (i = 0; i < BUF_SIZE - 1 - tau; i++) {
        sum += (audio_buf[i] >> 6) * (audio_buf[i] >> 6) + (audio_buf[i + tau] >> 6) * (audio_buf[i + tau] >> 6);
    }

    return sum;
}               

//#error Remember to define these pins before you run! // This assume you only have these 3 pins for I2S.
#define I2S_WS_PIN 4
#define I2S_SD_PIN 5
#define I2S_SCK_PIN 2
// #define SCK_PIN 2      //Serial clock for i2s
// #define LRCK_PIN 4     //Word select clock
// #define SDIN_PIN 5    //Data Input


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
	// while (!data_ready_flag)
	// {
	// 	k_sleep(K_MSEC(1));
    // //printk("waiting...\n");
	// 	//Wait for data. Since we do not want I2S_DATA_BLOCK_WORDS amount of prints inside the interrupt.
	// }
  //printk("Printing sound...\n");
	//nrfx_i2s_stop();
	data_ready_flag = false;

	/** Print the raw data from the I2S microphone */
	
	for (int i = 0; i < BUF_SIZE; i++)
	{
    //shift by 18 from mic datasheet
		printk("%d, \n", audio_buf[i] >> 6); //The audio is automatically saved in audio_buffer by the interrupt
	}
	printk("\n\n");
	

// 	/**  Format the data from Adafruit I2S MEMS Microphone Breakout, then print it	*/

// 	// int64_t sum = 0;
// 	// int64_t words = I2S_DATA_BLOCK_WORDS;

// 	// for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
// 	// {
// 	// 	memcpy(tmp + i, audio_buffer + i, sizeof(uint32_t));
// 	// 	tmp[i] >>= 8;
// 	// 	sum += tmp[i];
// 	// }

// 	// int64_t mean = sum / words;
// 	// for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
// 	// {
// 	// 	tmp[i] -= mean;
// 	// 	printk("%d, ", tmp[i]);
// 	// 	k_sleep(K_MSEC(16));
// 	// }
// 	// printk("\n\n");
// 	// /**  End of formatted data print*/


	//printk("Predicted: %f\n", predict_freq());
	

}

nrfx_err_t get_sound_init()
{
	IRQ_DIRECT_CONNECT(I2S_IRQn, 0, i2s_isr_handler, 0);
	memset(&audio_buf, 0, sizeof(audio_buf));
	initial_buffers.p_rx_buffer = audio_buf;

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

  
	err_code = nrfx_i2s_start(&initial_buffers, BUF_SIZE, 0); //start recording
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

	printk("Min:-50072,Max:50072\n");

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