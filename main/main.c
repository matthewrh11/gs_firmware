#include <esp_log.h>
#include <driver/i2s.h>
#include <driver/i2c.h>
#include "board.h"

#include "ch_button_press.h"

#include "gs_wifi.h"

#include "my_i2s.h"
#include "my_es8388.h"

#include "es8388.h"

/*
 * Digital Filtering Code
 * You can implement your own digital filters (e.g. FIR / IIR / biquad / ...) here.
 * Please mind that you need to use seperate storage for left / right channels for stereo filtering.
 */
int32_t dummyfilter(int32_t x){
	return x;
}

void app_main(void){

	printf("[filter-dsp] Initializing wifi...\r\n");
	gs_wifi_init();
	//gs_wifi_connect("BELL512", "alllowercase");

	printf("[filter-dsp] Initializing audio codec via I2C...\r\n");

	if (my_es8388_init() != ESP_OK) {
		printf("[filter-dsp] Audio codec initialization failed!\r\n");
	}
	else {
		printf("[filter-dsp] Audio codec initialization OK\r\n");
	}

	printf("[filter-dsp] Initializing bypass button\r\n");
	button_isr_config();

	printf("[filter-dsp] Initializing input I2S...\r\n");
	my_i2s_init();

	printf("[filter-dsp] Initializing MCLK output...\r\n");
	mclk_init();

	/*******************/

	printf("[filter-dsp] Enabling Passthrough mode...\r\n");

	size_t i2s_bytes_read = 0;
	size_t i2s_bytes_written = 0;

	int32_t i2s_buffer_read[I2S_READLEN / sizeof(int32_t)];
	int32_t i2s_buffer_write[I2S_READLEN / sizeof(int32_t)];

	ESP_LOGW("bypass_state", "%d", get_bypass_state());

	// continuously read data over I2S, pass it through the filtering function and write it back
	while (true) {
		i2s_bytes_read = I2S_READLEN;
		i2s_read(I2S_NUM, i2s_buffer_read, I2S_READLEN, &i2s_bytes_read, 100);

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
			// passthrough all data
			for (uint32_t i = 0; i < i2s_bytes_read / 2; i += 1){
				i2s_buffer_write[i] = i2s_buffer_read[i];
				//ESP_LOGW("LEFT_READ", "index: %d, value: %d", i, i2s_buffer_read[i]);
			}
		}
		i2s_write(I2S_NUM, i2s_buffer_write, i2s_bytes_read, &i2s_bytes_written, 100);
	}
}

// END


#if (0)
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_mem.h"
#include "audio_common.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"
#include "filter_resample.h"
#include "board.h"

//#include "dsprunner.h"

#include "print_names.h"
#include "ch_button_press.h"
#include "gs_wifi.h"

static const char *TAG = "PLAY_MP3_FLASH";
/*
   To embed it in the app binary, the mp3 file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
extern const uint8_t adf_music_mp3_start[] asm("_binary_adf_music_mp3_start");
extern const uint8_t adf_music_mp3_end[]   asm("_binary_adf_music_mp3_end");

int mp3_music_read_cb(audio_element_handle_t el, char *buf, int len, TickType_t wait_time, void *ctx)
{
    static int mp3_pos;
    int read_size = adf_music_mp3_end - adf_music_mp3_start - mp3_pos;
    if (read_size == 0) {
        return AEL_IO_DONE;
    } else if (len < read_size) {
        read_size = len;
    }
    memcpy(buf, adf_music_mp3_start + mp3_pos, read_size);
    mp3_pos += read_size;
    return read_size;
}

void app_main(void)
{
	print_names();
	button_isr_config();
	gs_wifi_init();
	gs_wifi_connect("BELL512", "alllowercase");

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    audio_pipeline_handle_t pipeline;
    audio_element_handle_t i2s_stream_writer, mp3_decoder;
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "[ 1 ] Start audio codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[ 2 ] Create audio pipeline, add all elements to pipeline, and subscribe pipeline event");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[2.1] Create mp3 decoder to decode mp3 file and set custom read callback");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_decoder = mp3_decoder_init(&mp3_cfg);
    audio_element_set_read_cb(mp3_decoder, mp3_music_read_cb, NULL);

    ESP_LOGI(TAG, "[2.2] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_cfg.i2s_config.sample_rate = 48000;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[2.3] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, mp3_decoder, "mp3");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[2.4] Link it together [mp3_music_read_cb]-->mp3_decoder-->i2s_stream-->[codec_chip]");

    /**Zl38063 does not support 44.1KHZ frequency, so resample needs to be used to convert files to other rates.
     * You can transfer to 16kHZ or 48kHZ.
     */
#if (CONFIG_ESP_LYRATD_MSC_V2_1_BOARD || CONFIG_ESP_LYRATD_MSC_V2_2_BOARD)
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = 44100;
    rsp_cfg.src_ch = 2;
    rsp_cfg.dest_rate = 48000;
    rsp_cfg.dest_ch = 2;
    audio_element_handle_t filter = rsp_filter_init(&rsp_cfg);
    audio_pipeline_register(pipeline, filter, "filter");
    const char *link_tag[3] = {"mp3", "filter", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 3);
#else
    const char *link_tag[2] = {"mp3", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 2);
#endif
    ESP_LOGI(TAG, "[ 3 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[3.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[ 4 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    while (1) {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) mp3_decoder
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(mp3_decoder, &music_info);

            ESP_LOGI(TAG, "[ * ] Receive music info from mp3 decoder, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);

            audio_element_setinfo(i2s_stream_writer, &music_info);

            /* Es8388 and es8374 and es8311 use this function to set I2S and codec to the same frequency as the music file, and zl38063
             * does not need this step because the data has been resampled.*/
#if (CONFIG_ESP_LYRATD_MSC_V2_1_BOARD || CONFIG_ESP_LYRATD_MSC_V2_2_BOARD)
#else
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates , music_info.bits, music_info.channels);
#endif
            continue;
        }
        /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) {
            break;
        }
    }

    ESP_LOGI(TAG, "[ 5 ] Stop audio_pipeline");
    audio_pipeline_stop(pipeline);
    audio_pipeline_wait_for_stop(pipeline);
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, mp3_decoder);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Make sure audio_pipeline_remove_listener is called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    audio_pipeline_unregister(pipeline, mp3_decoder);
#if (CONFIG_ESP_LYRATD_MSC_V2_1_BOARD || CONFIG_ESP_LYRATD_MSC_V2_2_BOARD)
    audio_pipeline_unregister(pipeline, filter);
    audio_element_deinit(filter);
#endif
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(mp3_decoder);
}
#endif
