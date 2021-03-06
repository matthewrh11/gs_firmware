#include "gs_effect_parser.h"

#include "esp_log.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsmn.h"

#include "gs_effect_router.h"

static const char* TAG = "gs_effect_parser";

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}

void gs_effect_parser(char* message, int size){
	char* payload = strndup(message, size);
	jsmn_parser parser;
	jsmntok_t tok[64];
	char* effect = "";
	char* state_string = "";
	ESP_LOGI(TAG, "Payload: %s", payload);

	jsmn_init(&parser);

	int num_tok = jsmn_parse(&parser, payload, strlen(payload), tok, sizeof(tok)/sizeof(tok[0]));

	if (num_tok < 0) {
		ESP_LOGE(TAG, "Failed to parse JSON: %d\n", num_tok);
	}
	if (num_tok < 1 || tok[0].type != JSMN_OBJECT) {
		ESP_LOGE(TAG, "Object expected\n");
	}

	for (int i = 1; i < num_tok; i++){
		if (jsoneq(payload, &tok[i], "title") == 0) {
			effect = strndup(payload + tok[i + 1].start, tok[i + 1].end - tok[i + 1].start);
			ESP_LOGI(TAG, "effect: %s", effect);
			i++;
		}
		else if (jsoneq(payload, &tok[i], "active") == 0) {
			state_string = strndup(payload + tok[i + 1].start, tok[i + 1].end - tok[i + 1].start);
			ESP_LOGI(TAG, "active: %s", state_string);
			i++;
		}
		else {
			ESP_LOGE(TAG, "Unexpected key!");
		}
	}

	bool state = strncmp(state_string, "true", strlen("true")) == 0;

	gs_effect_router(effect, state);

}
