#include <esp_log.h>
#include <esp_task_wdt.h>
#include <driver/i2s.h>
#include <driver/i2c.h>
#include "board.h"

#include "soc/rtc_wdt.h"

#include "ch_button_press.h"

#include "gs_wifi.h"
#include "gs_aws_connect.h"

#include "my_i2s.h"
#include "my_es8388.h"
#include "es8388.h"

#include "gs_effect_main.h"


void app_main(void){

	printf("[filter-dsp] Initializing wifi...\r\n");
	gs_wifi_init();

	//gs_wifi_connect();
	//gs_wifi_connect("BELL266", "JillRach");
	gs_wifi_connect("BELL512", "alllowercase");

    vTaskDelay(5000 / portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(&aws_iot_task, "aws_iot_task", 9216, NULL, 5, NULL, 1);

	printf("[filter-dsp] Initializing audio codec via I2C...\r\n");

	if (my_es8388_init() != ESP_OK) {
		printf("[filter-dsp] Audio codec initialization failed!\r\n");
	}
	else {
		printf("[filter-dsp] Audio codec initialization OK\r\n");
	}

	printf("[filter-dsp] Initializing bypass button\r\n");
	button_isr_config();

	printf("[filter-dsp] Initializing input I2S...\r\n");
	my_i2s_init();

	printf("[filter-dsp] Initializing MCLK output...\r\n");
	mclk_init();

	printf("[filter-dsp] Enabling Passthrough mode...\r\n");
	// continuously read data over I2S, pass it through the filtering function and write it back
	while (true) {
		run_effects();
		//rtc_wdt_feed();
		//vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

// fin
