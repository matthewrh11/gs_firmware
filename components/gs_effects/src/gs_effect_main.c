#include "gs_effect_main.h"

#include <stdlib.h>

#include "ch_button_press.h"
#include "my_i2s.h"

#include "gs_effect_bypass.h"
#include "gs_effect_fuzz.h"
#include "gs_effect_tremolo.h"

#define TAG1 "here"

size_t i2s_bytes_read = 0;
size_t i2s_bytes_written = 0;

int32_t i2s_buffer_read[I2S_READLEN / sizeof(int32_t)];
int32_t i2s_buffer_write[I2S_READLEN / sizeof(int32_t)];

int32_t temp_buffer1[I2S_READLEN / sizeof(int32_t)];
int32_t temp_buffer2[I2S_READLEN / sizeof(int32_t)];

/*
 * Digital Filtering Code
 * You can implement your own digital filters (e.g. FIR / IIR / biquad / ...) here.
 * Please mind that you need to use seperate storage for left / right channels for stereo filtering.
 */

void run_effects(){

	// continuously read data over I2S, pass it through the filtering function and write it back
	i2s_bytes_read = I2S_READLEN;
	i2s_read(I2S_NUM, i2s_buffer_read, I2S_READLEN, &i2s_bytes_read, 100);

	// Do DSP stuff
	if (!get_bypass_state()){
		gs_tremolo_effect(i2s_buffer_read, i2s_bytes_read, i2s_buffer_write);
//		gs_fuzz_effect(temp_buffer1, i2s_bytes_read, temp_buffer2);
//		gs_fuzz_effect(temp_buffer2, i2s_bytes_read, temp_buffer1);
//		gs_fuzz_effect(temp_buffer1, i2s_bytes_read, i2s_buffer_write);
	}
	// Passthrough all data
	else {
		gs_bypass_effect(i2s_buffer_read, i2s_bytes_read, i2s_buffer_write);
	}

	// Write to I2S
	i2s_write(I2S_NUM, i2s_buffer_write, i2s_bytes_read, &i2s_bytes_written, 100);
}
