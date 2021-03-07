#include "gs_effect_router.h"

#include <string.h>
#include "gs_effect_tremolo.h"
#include "gs_effect_fuzz.h"


void gs_effect_router(char* effect, bool state) {
	if (strncmp(effect, "Tremolo", strlen("Tremolo")) == 0) {
		set_tremolo_state(state);
		return;
	}
	else if (strncmp(effect, "Fuzz", strlen("Fuzz")) == 0) {
		set_fuzz_state(state);
		return;
	}
}
