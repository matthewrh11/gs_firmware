#include "gs_effect_router.h"

#include <string.h>

#include "gs_effect_fuzz.h"
#include "gs_effect_overdrive.h"
#include "gs_effect_ring.h"
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
	else if (strncmp(effect, "Ring", strlen("Ring")) == 0) {
		set_ring_state(state);
		return;
	}
	else if (strncmp(effect, "OD", strlen("OD")) == 0) {
		set_od_state(state);
		return;
	}
}
