#include "gpio.h"

#include "driver/gpio.h"
#include "esp_err.h"
#include "hal/gpio_types.h"
#include <stdio.h>

#define BTN1 3

void btn1_handler()
{
    printf("PRESSED\n");
}

void setup_gpio()
{
    ESP_ERROR_CHECK(gpio_install_isr_service(0));


    //gpio_set_direction(BTN1, GPIO_MODE_INPUT);
    //gpio_set_pull_mode(BTN1, GPIO_PULLUP_ONLY);
    //gpio_set_intr_type(BTN1, GPIO_INTR_NEGEDGE);

    //gpio_isr_handler_add(BTN1, btn1_handler, NULL);
}
