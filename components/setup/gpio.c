#include "gpio.h"
#include "common.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include <stdio.h>

#define NUM_BTNS 4

#define BTN1 23
#define BTN2 22
#define BTN3 3
#define BTN4 21

gpio_num_t btn_pins[NUM_BTNS] = {BTN1, BTN2, BTN3, BTN4};

volatile bool btn_pressed[NUM_BTNS] = {false};

Direction btn_to_dir[] = {LEFT, DOWN, UP, RIGHT};

static void IRAM_ATTR btn_handler(void *arg)
{
    int idx = (int)arg;
    btn_pressed[idx] = true;
}

void setup_gpio()
{
    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    for (int i = 0; i < NUM_BTNS; i++) {
        gpio_set_direction(btn_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(btn_pins[i], GPIO_PULLUP_ONLY);
        gpio_set_intr_type(btn_pins[i], GPIO_INTR_NEGEDGE);
        gpio_isr_handler_add(btn_pins[i], btn_handler, (void *)i);
    }
}

void btn_task(void *arg)
{
    GpioParams *params = (GpioParams *)arg;
    QueueHandle_t scan_queue = params->scan_queue;

    esp_task_wdt_deinit();
    bool last_state[NUM_BTNS] = {false};

    Direction direction;

    while (1) {
        for (int i = 0; i < NUM_BTNS; i++) {
            if (btn_pressed[i]) {
                direction = btn_to_dir[i];

                printf("btn %d\n", btn_pins[i]);
                xQueueSend(scan_queue, (void *)&direction, 0);
                btn_pressed[i] = false;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main()
{
    setup_gpio();
    xTaskCreate(btn_task, "btn_task", 2048, NULL, 10, NULL);
}

