#include "gs_effect_bypass.h"

void gs_bypass_effect(int32_t *input_buffer, size_t input_bytes_read, int32_t *output_buffer){
	for (uint32_t i = 0; i < input_bytes_read / 2; i += 1){
		output_buffer[i] = input_buffer[i];
	}
}
