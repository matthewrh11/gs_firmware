#include "gs_effect_router.h"

#include <string.h>
#include "gs_effect_tremolo.h"


void gs_effect_router(char* effect, bool state) {
	if (strncmp(effect, "Tremolo", strlen("Tremolo")) == 0) {
		set_tremolo_state(state);
		return;
	}
}
