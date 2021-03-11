#include "gs_effect_ring.h"
#include "gs_effect_common_functions.h"

#include "ch_button_press.h"
#include "my_i2s.h"

#include "esp_log.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

//static const char* TAG = "gs_ring_effect";
#define f_c 5
#define sin_ref_arr_size SAMPLING_RATE/f_c

bool gs_ring_state = false; // IMPORTANT: have this set to false for start-up
//bool gs_ring_state = true; // for testing WITHOUT app

int16_t ring_max_x_odd = 0;
float ring_max_x_odd_fl = 0.0;

static float fc = (float)f_c;
static float fs = (float)SAMPLING_RATE;

float sin_ref[sin_ref_arr_size];
int ring_index = 0;

void ring_init(){
	for (uint32_t i = 0; i < sin_ref_arr_size; i += 1){
		sin_ref[i] = sin(2*i*M_PI*fc/fs);
	}
}

void set_ring_state(bool state){
	gs_ring_state = state;
}

void gs_ring_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer) {
	ring_max_x_odd = max_arr_odd(input_buffer, input_bytes_read/2);
	ring_max_x_odd_fl = (float) ring_max_x_odd;
	float temp;

	// Saturn's Ringo
	if (gs_ring_state){
		// Even data ring
		for (uint32_t i = 0; i < input_bytes_read/2; i += 2){
			output_buffer[i] = 0;
		}
		// Odd data ring
		for (uint32_t i = 1; i < input_bytes_read/2; i += 2){
			temp = input_buffer[i]*sin_ref[ring_index]/ring_max_x_odd_fl;
			output_buffer[i] = (int16_t)(temp*ring_max_x_odd);
			ring_index++;
			if (ring_index == ((fs/fc)-1)){
				ring_index = 0;
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
