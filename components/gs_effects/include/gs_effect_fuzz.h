#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void set_fuzz_state(bool state);

void trem_init();

void gs_fuzz_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer);

void set_fuzz_gain(float gain);

float get_fuzz_gain();
