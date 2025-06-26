#include "gpio.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "utils.h"
#include <stdio.h>

#define NUM_DIRS 4

#define BTN1 21
#define BTN2 3
#define BTN3 22
#define BTN4 23

#define BTN5 19
#define BTN6 18

volatile bool btn_pressed[NUM_DIRS + 2] = {false};
gpio_num_t btn_pins[NUM_DIRS + 2] = {BTN1, BTN2, BTN3, BTN4, BTN5, BTN6};
Direction btn_to_dir[NUM_DIRS] = {LEFT, DOWN, UP, RIGHT};

#define GPIO_STACK_SIZE 4096

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

    // Setup buttons
    for (int i = 0; i < NUM_DIRS + 2; i++) {
        gpio_set_direction(btn_pins[i], GPIO_MODE_INPUT);
        gpio_set_pull_mode(btn_pins[i], GPIO_PULLUP_ONLY);
        gpio_set_intr_type(btn_pins[i], GPIO_INTR_NEGEDGE);
        gpio_isr_handler_add(btn_pins[i], btn_handler, (void *)i);
    }

    // Setup LED
    gpio_set_direction(SCAN_LED, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(SCAN_LED, GPIO_PULLUP_DISABLE);
    gpio_set_intr_type(SCAN_LED, GPIO_INTR_DISABLE);
}

void gpio_task_code(void *params)
{
    GpioParams *gpio_params = (GpioParams *)params;
    QueueHandle_t direction_queue = gpio_params->direction_queue;
    QueueHandle_t scan_queue = gpio_params->scan_queue;
    State *state = gpio_params->state;
    Pos *pos = gpio_params->pos;

    Direction direction;

    TickType_t last_press_time = 0;
    static const TickType_t debounce_time = pdMS_TO_TICKS(2000);

    while (1) {
        TickType_t now = xTaskGetTickCount();

        if (now - last_press_time > debounce_time) {
            last_press_time = now;

            for (int i = 0; i < NUM_DIRS; i++) {
                if (btn_pressed[i]) {
                    btn_pressed[i] = false;
                    direction = btn_to_dir[i];
                    xQueueSend(direction_queue, (void *)&direction, 0);
                    printf("Pressed button %d\n", btn_pins[i]);
                }
            }

            if (btn_pressed[NUM_DIRS]) {
                btn_pressed[NUM_DIRS] = false;
                printf("Pressed button %d\n", btn_pins[NUM_DIRS]);
                int x = 1;
                xQueueSend(scan_queue, (void *)&x, 0);
            }

            if (btn_pressed[NUM_DIRS + 1]) {
                btn_pressed[NUM_DIRS + 1] = false;
                printf("Pressed button %d\n", btn_pins[NUM_DIRS + 1]);
                toggle_state(state);
                *pos = (Pos){0, 0};
            }
        }
    }
}

TaskHandle_t gpio_task_create(GpioParams *gpio_params)
{
    TaskHandle_t handle =
        xTaskCreateStatic(gpio_task_code, "gpio_task", GPIO_STACK_SIZE, gpio_params,
                          tskIDLE_PRIORITY, gpio_stack, &gpio_tcb);
    return handle;
}
