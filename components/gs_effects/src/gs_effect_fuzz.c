#include "gs_effect_fuzz.h"
#include "gs_effect_common_functions.h"

#include "ch_button_press.h"
#include "my_i2s.h"

#include "esp_log.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

//static const char* TAG = "gs_fuzz_effect";

bool gs_fuzz_state = false;

int32_t max_x = 0;
float max_z = 0;

static float fuzz(float x){
	return 0.0;
}

void set_fuzz_state(bool state){
	gs_fuzz_state = state;
}

void gs_fuzz_effect(int32_t *input_buffer, size_t input_bytes_read, int32_t *output_buffer) {
	//int32_t max_x = max(input_buffer, input_bytes_read/2);
	//float max_x_fl = (float) max_x;

	// Gimme the fuzz
	if (gs_fuzz_state) {
		// left channel filter
		for (uint32_t i = 0; i < input_bytes_read / 2; i += 2){
			output_buffer[i] = fuzz(input_buffer[i]);
		}
		// right channel filter
		for (uint32_t i = 1; i < input_bytes_read / 2; i += 2){
			output_buffer[i] = fuzz(input_buffer[i]);
		}
	}
	// Bypass effects, in -> out
	else {
		for (uint32_t i = 0; i < input_bytes_read / 2; i += 1){
			output_buffer[i] = input_buffer[i];
		}
	}
	return;
}


