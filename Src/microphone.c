#include <stdint.h>
#include <string.h>
#include <zephyr.h>
#include "microphone.h"
#include "nrfx_i2s.h"
#include <hal/nrf_pwm.h>
#include <hal/nrf_gpio.h>

// Audio input buffer. Automatically written to by I2S DMA.
int32_t audio_buf[BUF_SIZE];
nrfx_i2s_buffers_t initial_buffers;

ISR_DIRECT_DECLARE(i2s_isr_handler)
{
	nrfx_i2s_irq_handler();
	ISR_DIRECT_PM(); /* PM done after servicing interrupt for best latency */
	return 1;
}

// Called when ISR fires. Resets I2S DMA buffer pointer.
static void data_handler(nrfx_i2s_buffers_t const *p_released, uint32_t status)
{
    if (NRFX_I2S_STATUS_NEXT_BUFFERS_NEEDED == status)
    {
        nrfx_err_t err = nrfx_i2s_next_buffers_set(&initial_buffers);
        if (err != NRFX_SUCCESS)
        {
            printk("Error!, continuing running as if nothing happened, but you should probably investigate.\n");
        }
    }
}

// initialize the pin to be used with the microphone.
nrfx_err_t microphone_init(uint8_t LRCL_pin, uint8_t DIN_pin, uint8_t BCLK_pin, uint8_t BCLK_PWM_pin, uint8_t LRCL_PWM_Pin) {

    /**
     *  We do weird stuff here. Basically the microphone only supports 24 bit I2S
     *  mode with an LRCK that is 1/64 the frequency of MCK. However, nRF52 
     *  requires that MCK / LRCK is a multiple of the sample width (24). So, we
     *  cannot rely on our processor's I2S module to generate the proper I2S clock.
     *  
     *  To get around this, we can actually run the I2S module in 24-bit peripheral 
     *  mode, and generate both the MCK and LRCK from the nRF's PWM module! This
     *  has been tested and confirmed working.
     * 
    */

    // PWM initialization code:
    uint32_t out_pins[] = {BCLK_PWM_pin, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};
    static uint16_t pwm_seq[2] = {0x8003, 0x0004};
    nrf_pwm_sequence_t const seq = 
    {
        .values.p_common = pwm_seq,
        .length          = sizeof(pwm_seq)/sizeof(uint16_t),
        .repeats         = 0,
        .end_delay       = 0
    };

    nrf_gpio_cfg_output(BCLK_PWM_pin);
    nrf_pwm_pins_set(NRF_PWM0, out_pins);
    nrf_pwm_enable(NRF_PWM0);
    //2.048MHz, actually 2.26MHz
    nrf_pwm_configure(NRF_PWM0, NRF_PWM_CLK_16MHz, NRF_PWM_MODE_UP, 0x7);
    nrf_pwm_loop_set(NRF_PWM0, 0);
    nrf_pwm_decoder_set(NRF_PWM0, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
    nrf_pwm_sequence_set(NRF_PWM0, 0, &seq);
    NRF_PWM0->TASKS_SEQSTART[0] = 1;

    uint32_t out_pins2[] = {NRF_PWM_PIN_NOT_CONNECTED, LRCL_PWM_Pin, NRF_PWM_PIN_NOT_CONNECTED, NRF_PWM_PIN_NOT_CONNECTED};
    static uint16_t pwm_seq2[2] = {0x8090, 0x00C0};
    nrf_pwm_sequence_t const seq2 = 
    {
        .values.p_common = pwm_seq2,
        .length          = sizeof(pwm_seq2)/sizeof(uint16_t),
        .repeats         = 0,
        .end_delay       = 0
    };
    nrf_gpio_cfg_output(LRCL_PWM_Pin);
    nrf_pwm_pins_set(NRF_PWM1, out_pins2);
    nrf_pwm_enable(NRF_PWM1);
    //2.048MHz / 4
    nrf_pwm_configure(NRF_PWM1, NRF_PWM_CLK_16MHz, NRF_PWM_MODE_UP, 0x1C0);
    nrf_pwm_loop_set(NRF_PWM1, 0);
    nrf_pwm_decoder_set(NRF_PWM1, NRF_PWM_LOAD_COMMON, NRF_PWM_STEP_AUTO);
    nrf_pwm_sequence_set(NRF_PWM1, 0, &seq2);
    NRF_PWM1->TASKS_SEQSTART[0] = 1;
    // End PWM initialization code

    // I2C initialization code
    IRQ_DIRECT_CONNECT(I2S_IRQn, 0, i2s_isr_handler, 0);
	memset(&audio_buf, 0, sizeof(audio_buf));
	initial_buffers.p_rx_buffer = audio_buf;

	nrfx_i2s_config_t config =
		NRFX_I2S_DEFAULT_CONFIG(BCLK_pin, LRCL_pin,
								NRFX_I2S_PIN_NOT_USED,
								NRFX_I2S_PIN_NOT_USED, DIN_pin);

	config.mode = NRF_I2S_MODE_SLAVE;			//Special- see block comment above
	config.ratio = NRF_I2S_RATIO_64X;			//Does nothing- we are in peripheral mode
	config.sample_width = NRF_I2S_SWIDTH_24BIT; //Microphone is 24-bit data
	config.mck_setup = NRF_I2S_MCK_32MDIV31;	//Does nothing- we are in peripheral mode
	config.channels = NRF_I2S_CHANNELS_LEFT;	//Currently the prototype is wired as a left input
  	config.alignment = NRF_I2S_ALIGN_LEFT;      //Unclear, but right does not work (?)

    nrfx_err_t err_code = nrfx_i2s_init(&config, data_handler);
    if (err_code != NRFX_SUCCESS)
    {
        printk("I2S init error\n");
    }
    return err_code;
}

// Starts the microphone
nrfx_err_t microphone_start(void) {
    nrfx_err_t err_code;

    err_code = nrfx_i2s_start(&initial_buffers, BUF_SIZE, 0);
	if (err_code != NRFX_SUCCESS)
	{
		printk("I2S start error\n");
		return err_code;
	}

    return err_code;
}

// Stops the microphone
void microphone_stop(void) {
    nrfx_i2s_stop();
}

// Returns the audio buffer.
int32_t *get_audio_buffer() {
    return audio_buf;
}
