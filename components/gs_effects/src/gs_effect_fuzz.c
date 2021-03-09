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

bool gs_fuzz_state = false; // IMPORTANT: have this set to false for start-up
//bool gs_fuzz_state = true; // for testing without app

float fuzz_gain = 3.0;

int16_t fuzz_max_x_odd = 0;
float fuzz_max_x_odd_fl = 0.0;

int16_t fuzz_max_y_odd = 0;


static int16_t fuzz(int16_t x, float max_val){
	float q = (float)(x)*(fuzz_gain/max_val);
	float sign_neg_q = sign(-q);
	float z = sign_neg_q*(1-exp(sign_neg_q*q));
	int16_t z_int = (int16_t)(z*max_val);
	return z_int;
}

void set_fuzz_state(bool state){
	gs_fuzz_state = state;
}

void gs_fuzz_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer) {
	fuzz_max_x_odd = max_arr_odd(input_buffer, input_bytes_read/2);
	fuzz_max_x_odd_fl = (float) fuzz_max_x_odd;

	// Gimme the fuzz
	if (gs_fuzz_state && fuzz_max_x_odd != 0){
		// even channel fuzz
		for (uint32_t i = 0; i < input_bytes_read/2; i += 2){
			output_buffer[i] = 0;
		}
		// odd channel fuzz
		for (uint32_t i = 1; i < input_bytes_read / 2; i += 2){
			output_buffer[i] = fuzz(input_buffer[i], fuzz_max_x_odd_fl);
		}
//		fuzz_max_y_odd = max_arr_odd(output_buffer, input_bytes_read/2);
//		for (uint32_t i = 1; i < input_bytes_read / 2; i += 2){
//			output_buffer[i] = output_buffer[i]/fuzz_max_y_odd;
//		}
	}
	// Bypass effects, in -> out
	else {
		for (uint32_t i = 0; i < input_bytes_read / 2; i += 1){
			output_buffer[i] = input_buffer[i];
		}
	}
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


