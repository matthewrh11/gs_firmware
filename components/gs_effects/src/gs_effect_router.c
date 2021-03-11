#include "gs_effect_router.h"

#include <string.h>

#include "gs_effect_flanger.h"
#include "gs_effect_fuzz.h"
#include "gs_effect_tremolo.h"


void gs_effect_router(char* effect, bool state) {
	if (strncmp(effect, "Tremolo", strlen("Tremolo")) == 0) {
		set_tremolo_state(state);
		return;
	}
	else if (strncmp(effect, "Fuzz", strlen("Fuzz")) == 0) {
		set_fuzz_state(state);
		return;
	}
	else if (strncmp(effect, "Flanger", strlen("Flanger")) == 0) {
		set_fuzz_state(state);
		return;
	}
}
