#include <esp_log.h>
#include <driver/i2s.h>
#include <driver/i2c.h>
#include "board.h"

#include "es8388.h"

esp_err_t set_mic_gain(es_mic_gain_t gain);

//static esp_err_t es_write_reg(uint8_t slave_add, uint8_t reg_add, uint8_t data);

void my_es8388_read_registers();

esp_err_t my_es8388_init();

