/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "ch_button_press.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define BYPASS_PIN     13 // Turn ON SW5 on the DIP selector
#define GPIO_INPUT_PIN_SEL  (1ULL<<BYPASS_PIN)
#define ESP_INTR_FLAG_DEFAULT 0

uint8_t bypass_state = 1;

static xQueueHandle gpio_evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg){
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void button_isr_task(void* arg){
    uint32_t io_num;
    for(;;) {
        if (xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
        	//ESP_LOGW("gpio_level", "%u", gpio_get_level(BYPASS_PIN));
        	//uint32_t t_start = esp_log_timestamp();
        	//while ((esp_log_timestamp()-t_start) < 50){
        	//}
        	//ESP_LOGE("gpio_level", "%u", gpio_get_level(BYPASS_PIN));
        	//if (gpio_get_level(BYPASS_PIN)){
                toggle_bypass_state();
                ESP_LOGE("bypass_state", "%u", bypass_state);
        	//}
        }
    }
}

void button_isr_config(void){
	ESP_LOGW("BYPASS_CONFIG", "start");
    gpio_config_t io_conf;

    //interrupt of rising edge
    //io_conf.intr_type = GPIO_INTR_HIGH_LEVEL;
    //io_conf.intr_type = GPIO_INTR_ANYEDGE;
    //io_conf.intr_type = GPIO_INTR_LOW_LEVEL;
    //io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.intr_type = GPIO_INTR_POSEDGE;

    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-down mode
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    //disable pull-up mode
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(button_isr_task, "button_isr_task", 2048, NULL, 10, NULL);

    // Install gpio isr service
    // LEAVE COMMENTED if GPIO isr service is already installed - this is done during codec initialization
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(BYPASS_PIN, gpio_isr_handler, (void*) BYPASS_PIN);
    ESP_LOGW("BYPASS_CONFIG", "finish");

    /* TEST START
    int cnt = 0;
    while(1) {
        ESP_LOGI("cnt", "%d", cnt++);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    TEST END */
}

void toggle_bypass_state(void){
	bypass_state = bypass_state^1;
}

void set_bypass_state(uint8_t new_state){
	if (new_state != 0){
		bypass_state = 1;
	}
	else{
		bypass_state = 0;
	}
}

uint8_t get_bypass_state(){
	return bypass_state;
}



