#include "gs_effect_main.h"

#include "ch_button_press.h"
#include "my_i2s.h"

#include "gs_effect_tremolo.h"

#define TAG1 "here"

size_t i2s_bytes_read = 0;
size_t i2s_bytes_written = 0;

int32_t i2s_buffer_read[I2S_READLEN / sizeof(int32_t)];
int32_t i2s_buffer_write[I2S_READLEN / sizeof(int32_t)];

// DELETE THIS AFTER
/*
 * Digital Filtering Code
 * You can implement your own digital filters (e.g. FIR / IIR / biquad / ...) here.
 * Please mind that you need to use seperate storage for left / right channels for stereo filtering.
 */
int32_t dummyfilter(int32_t x){
	return x;
}

void run_effects(){
	// continuously read data over I2S, pass it through the filtering function and write it back
	i2s_bytes_read = I2S_READLEN;
	i2s_read(I2S_NUM, i2s_buffer_read, I2S_READLEN, &i2s_bytes_read, 100);
	//ESP_LOGW(TAG1, "3");

	// Do DSP effect stuff
	if (!get_bypass_state()){
		// left channel filter
		for (uint32_t i = 0; i < i2s_bytes_read / 2; i += 2){
			i2s_buffer_write[i] = dummyfilter(i2s_buffer_read[i]);
			//ESP_LOGW("LEFT_READ", "index: %d, value: %d", i, i2s_buffer_read[i]);
		}

		// right channel filter
		for (uint32_t i = 1; i < i2s_bytes_read / 2; i += 2){
			i2s_buffer_write[i] = dummyfilter(i2s_buffer_read[i]);
			//ESP_LOGE("RIGHT_READ", "index: %d, value: %d", i, i2s_buffer_read[i]);
		}
	}
	// Bypass effects, in -> out
	else {
		//ESP_LOGW(TAG1, "3");
		// passthrough all data
		for (uint32_t i = 0; i < i2s_bytes_read / 2; i += 1){
			i2s_buffer_write[i] = i2s_buffer_read[i];
			//ESP_LOGW("LEFT_READ", "index: %d, value: %d", i, i2s_buffer_read[i]);
		}
	}
	//ESP_LOGW(TAG1, "4");
	i2s_write(I2S_NUM, i2s_buffer_write, i2s_bytes_read, &i2s_bytes_written, 100);
}
