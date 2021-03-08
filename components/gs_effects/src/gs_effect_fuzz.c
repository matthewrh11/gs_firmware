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

float gain = 0.8;
#define f_c 5
#define trem_arr_size SAMPLING_RATE/f_c

//bool gs_fuzz_state = false;
bool gs_fuzz_state = true;

float fuzz_gain = 5.0;

int16_t max_x_right = 0;
float max_x_right_fl = 0.0;

int16_t max_y_left = 0;
int16_t max_y_right = 0;

float fc = (float)f_c;
float fs = (float)SAMPLING_RATE;
float trem[trem_arr_size];
int trem_index = 0;


static int16_t fuzz(int16_t x, float max_val){
#if(0)
	x = (int16_t)(0.5*x);
	return x;
#else
	float q = (float)(x)*(fuzz_gain/max_val);
	float sign_neg_q = sign((int)(-1*q));
	float z = sign_neg_q*(1-exp(sign_neg_q*q));
	int16_t z_int = (int16_t)(z*max_val);
	return z_int;
#endif
}

void trem_init(){
	for (uint32_t i = 0; i < trem_arr_size; i += 1){
		// wtf is this shit
		trem[i] = 1.0+gain*sin(2*i*M_PI*fc/fs);
//		ESP_LOGI("trem", "%f, i: %d", trem[i], i);
	}
}

void set_fuzz_state(bool state){
	gs_fuzz_state = state;
}

void gs_fuzz_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer) {
	max_x_right = max_right(input_buffer, input_bytes_read/2);
	max_x_right_fl = (float) max_x_right;
	float temp;

	// Gimme the fuzz
	if (gs_fuzz_state){

		// Left channel fuzz
		for (uint32_t i = 0; i < input_bytes_read/2; i += 2){
			output_buffer[i] = 0; //input_buffer[i]; //(int16_t)((float)(input_buffer[i])*trem[i]);
			//ESP_LOGI("LEFT", "%d, i: %d", output_buffer[i], i);
		}
		// Right channel fuzz
		for (uint32_t i = 1; i < input_bytes_read / 2; i += 2){
			//output_buffer[i] =(int16_t)((float)(input_buffer[i])/(float)(max_x_right)*trem[trem_index]);
			temp = input_buffer[i]*trem[trem_index]/(max_x_right_fl);
			output_buffer[i] = (int16_t)(temp*(max_x_right));

//			ESP_LOGI("Tr", "%f, i: %d", trem[trem_index], trem_index);
//			ESP_LOGE("Tp", "%f, i: %d", temp, i);
//			ESP_LOGW("O1", "%d, i: %d", output_buffer[i], i);
//			ESP_LOGI("I ", "%d, i: %d", input_buffer[i], i);

			trem_index++;
			if (trem_index == ((SAMPLING_RATE/fc)-1)){
				trem_index = 0;
			}
			//output_buffer[i] = fuzz(input_buffer[i], max_x_right_fl); //input_buffer[i]; //(int32_t)((float)(input_buffer[i])*trem[i]);
		}


		max_y_right = max_right(output_buffer, input_bytes_read/2);
		for (uint32_t i = 1; i < input_bytes_read / 2; i += 2){
			//output_buffer[i] = output_buffer[i]/max_y_right;

			//ESP_LOGI("I ", "%d, i: %d", input_buffer[i], i);
			//ESP_LOGW("O2", "%d, i: %d", output_buffer[i], i);
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


