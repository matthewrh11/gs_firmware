#include <esp_log.h>
#include <driver/i2s.h>
#include <driver/i2c.h>
#include "board.h"

#define I2S_NUM I2S_NUM_0
#define I2S_READLEN 50 * 4

void my_i2s_init();

void mclk_init();


