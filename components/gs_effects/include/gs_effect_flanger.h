#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void set_flanger_state(bool state);

void flanger_init();

void gs_flanger_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer);
