/* gs_effect_main
 *
 * Setup I2S, read, do desired effects, and write
 *
 */
#include <driver/i2s.h>
#include "my_i2s.h"

// DELETE THIS AFTER
int32_t dummyfilter(int32_t x);

void run_effects();
