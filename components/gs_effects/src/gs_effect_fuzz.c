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

float fuzz_gain = 5.0;

int32_t max_x_left = 0;
float max_x_left_fl = 0.0;

int32_t max_x_right = 0;
float max_x_right_fl = 0.0;

int32_t max_y_left = 0;
int32_t max_y_right = 0;


float trem[I2S_READLEN/sizeof(int16_t)];
float gain = 0.8;
float fc = 2.296;


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
	for (uint32_t i = 0; i < 100; i += 1){
		trem[i] = (1+gain*sin(2*M_PI*i*(fc/44100)));
	}
}

void set_fuzz_state(bool state){
	gs_fuzz_state = state;
}

void gs_fuzz_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer) {
//	max_x_left = max_left(input_buffer, input_bytes_read/2);
//	max_x_left_fl = (float) max_x_left;
//
	max_x_right = max_right(input_buffer, input_bytes_read/2);
	max_x_right_fl = (float) max_x_right;


	//ESP_LOGI("input_bytes_read = ", "%d", input_bytes_read);
	if (gs_fuzz_state){

		// Left channel fuzz
		for (uint32_t i = 0; i < input_bytes_read/2; i += 2){
			output_buffer[i] = 0; //input_buffer[i]; //(int16_t)((float)(input_buffer[i])*trem[i]);
			//ESP_LOGI("LEFT", "%d, i: %d", output_buffer[i], i);
		}
		// Right channel fuzz
		for (uint32_t i = 1; i < input_bytes_read / 2; i += 2){
			output_buffer[i] = fuzz(input_buffer[i], max_x_right_fl); //input_buffer[i]; //(int32_t)((float)(input_buffer[i])*trem[i]);
			//ESP_LOGW("RIGHT", "%d, i: %d", output_buffer[i], i);
		}

	}



//	// Gimme the fuzz
//	if (gs_fuzz_state && max_x_left != 0) {
//		// Left channel fuzz
//		for (uint32_t i = 0; i < input_bytes_read / 2; i += 2){
//			output_buffer[i] = fuzz(input_buffer[i], max_x_left_fl);
//			//ESP_LOGI("output_buffer[i] = ", "%d", output_buffer[i]);
//		}
//		// Right channel fuzz
//			for (uint32_t i = 1; i < input_bytes_read / 2; i += 2){
//				output_buffer[i] =  fuzz(input_buffer[i], max_x_right_fl);
//				//ESP_LOGI("output_buffer[i] = ", "%d", output_buffer[i]);
//			}
//
//		// Normalize output
//		// Left
////		int32_t max_y_left = max_left(output_buffer, input_bytes_read/2);
////		for (uint32_t i = 0; i < input_bytes_read / 2; i += 2){
////			output_buffer[i] *= output_buffer[i]/max_y_left;
////		}
//		// Right
//		int32_t max_y_right = max_right(output_buffer, input_bytes_read/2);
//		for (uint32_t i = 1; i < input_bytes_read / 2; i += 2){
//			output_buffer[i] *= output_buffer[i]/max_y_right;
//		}
//
//	}
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


