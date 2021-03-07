#include "gs_effect_fuzz.h"
#include "gs_effect_common_functions.h"

#include "soc/rtc_wdt.h"

#include "ch_button_press.h"
#include "my_i2s.h"

#include "esp_log.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

//static const char* TAG = "gs_fuzz_effect";

//bool gs_fuzz_state = false;
bool gs_fuzz_state = true;

float fuzz_gain = 10.0;

int32_t max_x = 0;
float max_x_fl = 0.0;
float max_z_fl = 0;

static int32_t fuzz(float x){
	float q = x*(fuzz_gain/max_x_fl);
	float sign_neg_q = sign((int)(-1*q));
	float z = sign_neg_q*(1-exp(sign_neg_q*q));
	int32_t z_int = (int32_t)(z*max_x_fl);
	return z_int;
}

void set_fuzz_state(bool state){
	gs_fuzz_state = state;
}

void gs_fuzz_effect(int32_t *input_buffer, size_t input_bytes_read, int32_t *output_buffer) {
	max_x = max(input_buffer, input_bytes_read/2);
	max_x_fl = (float) max_x;

	//ESP_LOGE("max_x = ", "%d", max_x);
	//ESP_LOGW("max_x_fl = ", "%f", max_x_fl);

	// Gimme the fuzz
	if (gs_fuzz_state && max_x != 0) {
		//rtc_wdt_feed();
		// left and right channel filter, does not depend on an average or previous values so do both at the same time
		for (uint32_t i = 0; i < input_bytes_read / 2; i += 1){
			output_buffer[i] = fuzz(input_buffer[i]);
			//ESP_LOGI("output_buffer[i] = ", "%d", output_buffer[i]);
		}

		// normalize output
		int32_t max_y = max(output_buffer, input_bytes_read/2);
		//ESP_LOGW("max_y = ", "%d", max_y);
		for (uint32_t i = 0; i < input_bytes_read / 2; i += 1){
			output_buffer[i] *= output_buffer[i]/max_y;
		}

	}
	// Bypass effects, in -> out
	else {
		for (uint32_t i = 0; i < input_bytes_read / 2; i += 1){
			output_buffer[i] = input_buffer[i];
		}
	}
	//ESP_LOGW("fuzz", "inga");
	return;
}

void set_fuzz_gain(float gain){
	if (gain < 1){
		fuzz_gain = 1;
	}
	else if (gain > 20){
		fuzz_gain = 20;
	}
	else {
		fuzz_gain = gain;
	}
}

float get_fuzz_gain(){
	return fuzz_gain;
}


