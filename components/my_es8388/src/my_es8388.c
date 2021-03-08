#include "my_es8388.h"

#include <esp_log.h>
#include <driver/i2s.h>
#include <driver/i2c.h>
#include "i2c_bus.h"
#include "board.h"

#include "es8388.h"

/*
 * Basic I2S and I2C Configuration
 */

static const char *ES_TAG = "ES8388_DRIVER";
static i2c_bus_handle_t i2c_handle;

#define I2C_NUM I2C_NUM_0
#define ES8388_ADDR 0x20

#define ES_ASSERT(a, format, b, ...) \
    if ((a) != 0) { \
        ESP_LOGE(ES_TAG, format, ##__VA_ARGS__); \
        return b;\
    }

/**
 * @param gain: see es_mic_gain_t
 *
 * @return
 *     - (-1) Parameter error
 *     - (0)   Success
 */
esp_err_t set_mic_gain(es_mic_gain_t gain)
{
    esp_err_t gain_n;
    gain_n = (int)gain / 3;
    gain_n = (gain_n << 4) + gain_n;
    return gain_n;
}

/*
 * ES8388 Configuration Code
 * Configure ES8388 audio codec over I2C for AUX IN input and headphone jack output
 */
static esp_err_t es_write_reg(uint8_t slave_add, uint8_t reg_add, uint8_t data){
	esp_err_t res = ESP_OK;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	res |= i2c_master_start(cmd);
	res |= i2c_master_write_byte(cmd, slave_add, 1 /*ACK_CHECK_EN*/);
	res |= i2c_master_write_byte(cmd, reg_add, 1 /*ACK_CHECK_EN*/);
	res |= i2c_master_write_byte(cmd, data, 1 /*ACK_CHECK_EN*/);
	res |= i2c_master_stop(cmd);
	res |= i2c_master_cmd_begin(0, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	return res;
}

static esp_err_t es_read_reg(uint8_t reg_add, uint8_t *p_data)
{
    return i2c_bus_read_bytes(i2c_handle, ES8388_ADDR, &reg_add, sizeof(reg_add), p_data, 1);
}

void my_es8388_read_registers(){
    for (int i = 0; i < 50; i++) {
        uint8_t reg = 0;
        es_read_reg(i, &reg);
        ets_printf("%x: %x\n", i, reg);
    }
}

static int i2c_init(){
    int res;
    i2c_config_t es_i2c_cfg = {
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000
    };
    res = get_i2c_pins(I2C_NUM_0, &es_i2c_cfg);
    ES_ASSERT(res, "getting i2c pins error", -1);
    i2c_handle = i2c_bus_create(I2C_NUM_0, &es_i2c_cfg);
    return res;
}

esp_err_t my_es8388_init(){
	esp_err_t res = ESP_OK;
#if(1)
	i2c_config_t i2c_config = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = GPIO_NUM_18,
			.sda_pullup_en = true,
			.scl_io_num = GPIO_NUM_23,
			.scl_pullup_en = true,
			.master.clk_speed = 100000
		};

	res |= i2c_param_config(I2C_NUM, &i2c_config);
	res |= i2c_driver_install(I2C_NUM, i2c_config.mode, 0, 0, 0);

	/* mute DAC during setup, power up all systems, slave mode */
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL3, 0x04);
	res |= es_write_reg(ES8388_ADDR, ES8388_CONTROL2, 0x50);
	res |= es_write_reg(ES8388_ADDR, ES8388_CHIPPOWER, 0x00);
	res |= es_write_reg(ES8388_ADDR, ES8388_MASTERMODE, 0x00);

	/* power up DAC and enable only LOUT1 / ROUT1, ADC sample rate = DAC sample rate */
	res |= es_write_reg(ES8388_ADDR, ES8388_DACPOWER, 0x30);
	res |= es_write_reg(ES8388_ADDR, ES8388_CONTROL1, 0x12);

	/* DAC I2S setup: 16 bit word length, I2S format; MCLK / Fs = 256*/
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL1, 0x18);
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL2, 0x02);

	/* DAC to output route mixer configuration */
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL16, 0x00);
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL17, 0x90);
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL20, 0x90);

	/* DAC and ADC use same LRCK, enable MCLK input; output resistance setup */
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL21, 0x80);
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL23, 0x00);

	/* DAC volume control: 0dB (maximum, unattenuated)  */
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL5, 0x00);
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL4, 0x00);

	/* power down ADC while configuring; volume: +9dB for both channels */
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCPOWER, 0xff);
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL1, 0x33);

	/* select LINPUT2 / RINPUT2 as ADC input; stereo; 16 bit word length, format right-justified, MCLK / Fs = 256 */
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL2, 0x50);
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL3, 0x00);
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL4, 0x0e);
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL5, 0x02);

	/* set ADC volume */
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL8, 0x20);
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL9, 0x20);

	/* set LOUT1 / ROUT1 volume: 0dB (unattenuated) */
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL24, 0x1e);
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL25, 0x1e);

	/* power up and enable DAC; power up ADC (no MIC bias) */
	res |= es_write_reg(ES8388_ADDR, ES8388_DACPOWER, 0x3c);
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL3, 0x00);
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCPOWER, 0x09);
#else
	res = i2c_init(); // ESP32 in master mode

	// mute DAC during setup, power up all systems, slave mode
	//
	// 0.5 dB per 4 LRCK digital volume control ramp rate, disabled digital volume control soft ramp, DAC unmuted
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL3, 0x04);
	// LPVrefBuf in low power mode, all others in normal mode
	res |= es_write_reg(ES8388_ADDR, ES8388_CONTROL2, 0x50);
	// all power on, ADC and DAC analog reference power on
	res |= es_write_reg(ES8388_ADDR, ES8388_CHIPPOWER, 0x00);
	// slave, MCLK not divide, non-inverted BCLK, BCLK generated from clock table
	res |= es_write_reg(ES8388_ADDR, ES8388_MASTERMODE, 0x00);

	// power up DAC and enable only LOUT1 / ROUT1, ADC sample rate = DAC sample rate
	//
	// L/R DAC power on, LOUT1/ROUT1 enabled, LOUT2/ROUT2 disabled
	res |= es_write_reg(ES8388_ADDR, ES8388_DACPOWER, 0x30);
	// ADC and DAC have same fs, 500 kohm divider enabled
	res |= es_write_reg(ES8388_ADDR, ES8388_CONTROL1, 0x12);

	//
	// DAC RESOLUTION (1) and SAMPLING RATE (2)
	//
	// DAC I2S setup: bit word length, I2S format; MCLK / Fs
	//
	// L/R normal polarity, 16-bit data word
	//res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL1, 0x18);
	// single speed mode, DAC MCLK to sampling frequency ratio = 128
	//res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL2, 0x02);

	// DAC to output route mixer configuration
	//
	// Left input for output mix = LIN1, Right input for output mix = RIN1
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL16, 0x00);
	// left DAC to left mixer enable, LIN signal to left mixer gain = 0 dB
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL17, 0x90);
	// right DAC to left mixer enable, RIN signal to left mixer gain = 0 dB
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL20, 0x90);

	// DAC and ADC use same LRCK, enable MCLK input; output resistance setup
	//
	// DACLRC and ADCLRC same, use DAC LRCK, no offset, ADC and DAC DLL on
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL21, 0x80);
	// 1.5k VREF to analog output resistance
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL23, 0x00);

	// DAC volume control: 0dB (maximum, unattenuated)
	//
	// RDACVOL: 0 dB attenuation (0.5 dB incremental from 0 to –96 dB -> 0x00 to 0xC0)
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL5, 0x00);
	// LDACVOL: 0 dB attenuation (0.5 dB incremental from 0 to –96 dB -> 0x00 to 0xC0)
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL4, 0x00);

	// power down ADC while configuring; volume: +24 dB for both channels
	//
	// all power off for ADC
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCPOWER, 0xff);
	// Left channel PGA gain = +24 dB, Right channel PGA gain = 0 dB = max gain
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL1, 0x80);

	//
	// ADC RESOLUTION (4) and SAMPLING RATE (5)
	//
	// select LINPUT2 / RINPUT2 as ADC input; stereo; bit word length, format right-justified, MCLK / Fs
	//
	// LINPUT2 / RINPUT2 as ADC input
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL2, 0x50);
	// stereo input
	//res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL3, 0x80);
	// mono input
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL3, 0x08);
	// mono output
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL7, 0x20);
	// left data = left ADC, right data = right ADC, left and right normal polarity
	// 16-bit serial audio data word length, right justify serial audio data format
	//res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL4, 0x0e);
	// single speed mode, ADC MCLK to sampling frequency ratio = 256
	//res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL5, 0x02);

	// resolution
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL1, 0x02);
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL4, 0x01);
	// sampling rate
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL2, 0x21);
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL5, 0x21);

	// set ADC volume
	//
	// LADCVOL: -1 dB attenuation (0.5 dB incremental from 0 to –96 dB -> 0x00 to 0xC0)
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL8, 0x00);
	// RADCVOL: -1 dB attenuation (0.5 dB incremental from 0 to –96 dB -> 0x00 to 0xC0)
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCCONTROL9, 0xc0);

	// set LOUT1 / ROUT1 volume: 0dB (unattenuated)
	//
	// LOUT1VOL = +4.5 dB
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL24, 0x21);
	// ROUT1VOL = -45 dB
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL25, 0x00);

	// IF YOU WANT BOTH HEADPHONES //
	// ROUT1VOL = +4.5 dB
	//res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL25, 0x21);

	// power up and enable DAC; power up ADC (no MIC bias)
	//
	// LOUT1/ROUT1 and LOUT2/ROUT2 enabled
	res |= es_write_reg(ES8388_ADDR, ES8388_DACPOWER, 0x3c);
	// un-mute DAC
	res |= es_write_reg(ES8388_ADDR, ES8388_DACCONTROL3, 0x08);
	// microphone bias power down (high impedance output, default), int1 low power
	res |= es_write_reg(ES8388_ADDR, ES8388_ADCPOWER, 0x09);
#endif

	return res;
}
