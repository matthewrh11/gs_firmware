#include "gs_effect_tremolo.h"
#include "gs_effect_common_functions.h"

#include "soc/rtc_wdt.h"

#include "ch_button_press.h"
#include "my_i2s.h"

#include "esp_log.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

//static const char* TAG = "gs_tremolo_effect";
#define f_c 5
#define trem_arr_size SAMPLING_RATE/f_c

//bool gs_tremolo_state = false; // IMPORTANT: have this set to false for start-up
bool gs_tremolo_state = true; // for testing without app

int16_t trem_max_x_odd = 0;
float trem_max_x_odd_fl = 0.0;
float trem_max_trem;
int16_t trem_max_y_odd = 0;


float gain = 0.8;
float fc = (float)f_c;
float fs = (float)SAMPLING_RATE;
float trem[trem_arr_size];
int trem_index = 0;

void tremolo_init(){
	for (uint32_t i = 0; i < trem_arr_size; i += 1){
		trem[i] = 1.0 + gain*sin(2*i*M_PI*fc/fs);
	}
	trem_max_trem = 1.0 + gain;
}

void set_tremolo_state(bool state){
	gs_tremolo_state = state;
}

void gs_tremolo_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer) {
	trem_max_x_odd = max_arr_odd(input_buffer, input_bytes_read/2);
	trem_max_x_odd_fl = (float) trem_max_x_odd;
	float temp;

	// Tremololo
	if (gs_tremolo_state){
		// Even data tremololo
		for (uint32_t i = 0; i < input_bytes_read/2; i += 2){
			output_buffer[i] = 0;
		}
		// Odd data tremololo
		for (uint32_t i = 1; i < input_bytes_read / 2; i += 2){
			temp = input_buffer[i]*trem[trem_index]/(trem_max_x_odd_fl);
			output_buffer[i] = (int16_t)(temp*(trem_max_x_odd)/trem_max_trem);
			trem_index++;
			if (trem_index == ((SAMPLING_RATE/fc)-1)){
				trem_index = 0;
			}
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
