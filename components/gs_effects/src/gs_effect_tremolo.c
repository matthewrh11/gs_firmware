#include "gs_effect_tremolo.h"


#include "esp_log.h"
#include <string.h>

static const char* TAG = "gs_tremolo_effect";

bool gs_tremolo_state = false;
float gain = 0.8;
float fc = 5;

void set_tremolo_state(bool state){
	gs_tremolo_state = state;
}

/*
 * TODO: change return type, add input param.
 */
void gs_tremolo_effect(int32_t *input_buffer, size_t input_bytes_read, int32_t *output_buffer) {
	if (gs_tremolo_state) {
		float trem[input_bytes_read/2];
		for (uint32_t i = 0; i < input_bytes_read / 2; i += 1){
			trem[i] = (1+gain*sin(2*M_PI*i*(fc/96000)));
			output_buffer[i] = input_buffer[i]*trem[i];
		}
	}
	else {
		for (uint32_t i = 0; i < input_bytes_read / 2; i += 1){
			output_buffer[i] = input_buffer[i];
		}
	}
	return;
}





