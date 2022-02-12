#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void set_ring_state(bool state);

void ring_init();

void gs_ring_effect(int16_t *input_buffer, size_t input_bytes_read, int16_t *output_buffer);
