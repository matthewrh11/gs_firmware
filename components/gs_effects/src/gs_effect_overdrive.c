#include "gs_effect_overdrive.h"
#include "gs_effect_common_functions.h"

#include "ch_button_press.h"
#include "my_i2s.h"

#include "esp_log.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

//static const char* TAG = "gs_od_effect";

bool gs_od_state = false; // IMPORTANT: have this set to false for start-up
//bool gs_od_state = true; // for testing WITHOUT app

int16_t od_max_x_odd = 0;
float od_max_x_odd_fl = 0.0;
static float threshold = 1.0/3.0;


void set_od_state(bool state){
	gs_od_state = state;
}

void gs_od_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer) {
	od_max_x_odd = max_arr_odd(input_buffer, input_bytes_read/2);
	od_max_x_odd_fl = (float) od_max_x_odd;
	float temp;

	// OVERDRIVE
	if (gs_od_state && od_max_x_odd != 0){
		// Even data od
		for (uint32_t i = 0; i < input_bytes_read/2; i += 2){
			output_buffer[i] = 0;
		}
		// Odd data od
		for (uint32_t i = 1; i < input_bytes_read/2; i += 2){
			temp = input_buffer[i]/od_max_x_odd_fl;
			if (abs_float(temp) < threshold){
				temp *= 2.0;
			}
			else if (abs_float(temp) > 2*threshold){
				temp = sign(temp);
			}
			else {
				temp = sign(temp)*(3.0-pow(2.0-3.0*abs_float(temp),2.0))/3.0;
			}
			output_buffer[i] = (int16_t)(temp*od_max_x_odd);
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
