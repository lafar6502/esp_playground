#include <stddef.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "test_task.h"


void blink_task(void *pvParameter)
{
    gpio_config_t io_conf;
    //disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT; //GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = 1ULL << GPIO_OUTPUT_IO_0;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	printf("gp init");
	int cnt = 0;
	while(1)
	{
	    vTaskDelay(1000 / portTICK_PERIOD_MS);
	    gpio_set_level(GPIO_OUTPUT_IO_0, (cnt++) % 2);
            printf("gp %d - l%d\r\n", GPIO_OUTPUT_IO_0, gpio_get_level(GPIO_OUTPUT_IO_0));
	}
}

void initTestTask() {

	 xTaskCreate(blink_task,
                "blink_task",
                2000,
                NULL,
                1,
                NULL);
	printf("gpio task created");
}


