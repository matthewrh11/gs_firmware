#include "esp_log.h"
#include <stdbool.h>
#include <string.h>

static const char* TAG = "gs_tremolo_effect";

bool gs_tremolo_state = false;

void set_tremolo_state(bool state){
	gs_tremolo_state = state;
}

/*
 * TODO: change return type, add input param.
 */
void gs_tremolo_effect() {
	if (gs_tremolo_state) {
		ESP_LOGW(TAG, "here we would be adding tremolo!");
		return;
	}
	ESP_LOGW(TAG, "here we ARE NOT adding tremolo!");
	return;
}
