// Created by Hiscock

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/gpio.h"

//static void IRAM_ATTR gpio_isr_handler(void* arg);

//static void button_isr_task(void* arg);

void button_isr_config(void);

void toggle_bypass_state(void);

void set_bypass_state(uint8_t new_state);

uint8_t get_bypass_state();
