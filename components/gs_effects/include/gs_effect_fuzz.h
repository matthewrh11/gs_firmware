#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void set_fuzz_state(bool state);

void gs_fuzz_effect(int32_t *input_buffer, size_t input_bytes_read, int32_t *output_buffer);
