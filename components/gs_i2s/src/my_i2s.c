#include "my_i2s.h"

#include <esp_log.h>
#include <driver/i2s.h>
#include <driver/i2c.h>
#include "board.h"

void my_i2s_init(){

	i2s_config_t i2s_read_config = {
			.mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX,
			.sample_rate = 96000,
			.bits_per_sample = 24,
			.communication_format = I2S_COMM_FORMAT_I2S,
			.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
			.intr_alloc_flags = ESP_INTR_FLAG_LEVEL2,
			.dma_buf_count = 3,
			.dma_buf_len = I2S_READLEN,
			.use_apll = 1,
			.tx_desc_auto_clear = 1,
			.fixed_mclk = 0
	};

	i2s_pin_config_t i2s_read_pin_config = {
		.bck_io_num = GPIO_NUM_5,
		.ws_io_num = GPIO_NUM_25,
		.data_out_num = GPIO_NUM_26,
		.data_in_num = GPIO_NUM_35
	};


	i2s_driver_install(I2S_NUM, &i2s_read_config, 0, NULL);
	i2s_set_pin(I2S_NUM, &i2s_read_pin_config);
}

void mclk_init(){
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
	WRITE_PERI_REG(PIN_CTRL, 0xFFF0);
}
