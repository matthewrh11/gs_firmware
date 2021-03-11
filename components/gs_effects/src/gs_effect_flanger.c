#include "gs_effect_flanger.h"
#include "gs_effect_common_functions.h"

#include "ch_button_press.h"
#include "my_i2s.h"

#include "esp_log.h"
#include <stdbool.h>
#include <string.h>
#include <math.h>

//static const char* TAG = "gs_flanger_effect";
#define f_c 5
#define sin_ref_arr_size SAMPLING_RATE/f_c

//bool gs_flanger_state = false; // IMPORTANT: have this set to false for start-up
bool gs_flanger_state = true; // for testing WITHOUT app

int16_t flanger_max_x_odd = 0;
float flanger_max_x_odd_fl = 0.0;
float flanger_max_flanger;
int16_t flanger_max_y_odd = 0;


float max_time_delay = 0.0005669;
int max_sample_delay_div2;
static float fc = (float)f_c;
static float fs = (float)SAMPLING_RATE;
float amp = 0.7;
float sin_ref[sin_ref_arr_size];
int flanger_index = 0;

void flanger_init(){
	for (uint32_t i = 0; i < sin_ref_arr_size; i += 1){
		sin_ref[i] = sin(2*i*M_PI*fc/fs);
	}
#if(0)
	flanger_max_flanger = amp;
	max_sample_delay_div2 = (int)(max_time_delay*fs);
	ESP_LOGW("max_sample_delay_div2","%d",max_sample_delay_div2);
	if (max_sample_delay_div2%2 == 0){
		max_sample_delay_div2 += 1;
	}
	ESP_LOGW("max_sample_delay_div2","%d",max_sample_delay_div2);
#endif
}

void set_flanger_state(bool state){
	gs_flanger_state = state;
}

void gs_flanger_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer) {
	flanger_max_x_odd = max_arr_odd(input_buffer, input_bytes_read/2);
	flanger_max_x_odd_fl = (float) flanger_max_x_odd;
	float temp;

	float cur_sin;
	int cur_delay;

	// flanger
	if (gs_flanger_state){
		// Even data flanger
		for (uint32_t i = 0; i < input_bytes_read/2; i += 2){
			output_buffer[i] = 0;
		}
		for (uint32_t i = 1; i < input_bytes_read/2; i += 2){
			temp = input_buffer[i]*sin_ref[flanger_index]/flanger_max_x_odd_fl;
			//ESP_LOGI("temp","%f",temp);
			//ESP_LOGI("temp","%f",temp);
			output_buffer[i] = (int16_t)(temp*flanger_max_x_odd);
			//ESP_LOGE("I","%d",input_buffer[i]);
			//ESP_LOGE("O","%d",output_buffer[i]);
			flanger_index++;
			if (flanger_index == ((fs/fc)-1)){
				flanger_index = 0;
			}
		}

#if(0)
		// Odd data flanger
		for (uint32_t i = 1; i < (max_sample_delay_div2-1); i += 2){
			if (i == input_bytes_read / 2){
				break;
			}
			output_buffer[i] = input_buffer[i];
		}
		for (uint32_t i = max_sample_delay_div2; i < input_bytes_read/2; i += 2){
			if (i == input_bytes_read / 2){
				break;
			}
			cur_sin = abs_float(sin_ref[flanger_index]);
			ESP_LOGI("cur_sin","%f",cur_sin);
			cur_delay = ceil(cur_sin*max_sample_delay_div2);
			ESP_LOGW("cur_delay","%d",cur_delay);
			if (cur_delay%2 == 0){
				cur_delay -= 1;
				if (cur_delay < 0){
					cur_delay = 0;
				}
			}
			ESP_LOGW("cur_delay","%d",cur_delay);
			temp = amp*(input_buffer[i] + input_buffer[i-cur_delay])/flanger_max_x_odd_fl;
			ESP_LOGI("temp","%f",temp);
			output_buffer[i] = (int16_t)(temp*flanger_max_x_odd);///flanger_max_flanger);
			ESP_LOGE("I","%d",input_buffer[i]);
			ESP_LOGE("O","%d",output_buffer[i]);


		}
#endif
	}
	// Bypass effects, in -> out
	else {
		for (uint32_t i = 0; i < input_bytes_read / 2; i += 1){
			output_buffer[i] = input_buffer[i];
		}
	}
	return;
}
