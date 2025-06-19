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

#define BTN1 21
#define BTN2 3
#define BTN3 22
#define BTN4 23

volatile bool btn_pressed[NUM_BTNS] = {false};
gpio_num_t btn_pins[NUM_BTNS] = {BTN1, BTN2, BTN3, BTN4};
Direction btn_to_dir[] = {LEFT, DOWN, UP, RIGHT};

StaticTask_t gpio_tcb;
StackType_t gpio_stack[GPIO_STACK_SIZE];

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

void gpio_task_code(void *params)
{
    GpioParams *gpio_params = (GpioParams *)params;
    QueueHandle_t scan_queue = gpio_params->scan_queue;

    Direction direction;

    while (1) {
        for (int i = 0; i < NUM_BTNS; i++) {
            if (btn_pressed[i]) {
                direction = btn_to_dir[i];
                xQueueSend(scan_queue, (void *)&direction, 0);
                btn_pressed[i] = false;
                printf("Pressed button %d\n", btn_pins[i]);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

TaskHandle_t gpio_task_create(GpioParams *gpio_params)
{
    TaskHandle_t handle =
        xTaskCreateStatic(gpio_task_code, "gpio_task", GPIO_STACK_SIZE,
                          gpio_params, tskIDLE_PRIORITY, gpio_stack, &gpio_tcb);
    return handle;
}
