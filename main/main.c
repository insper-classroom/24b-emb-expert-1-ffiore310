/*
 * LED blink with FreeRTOS
 */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>
#include "hardware/adc.h"

#include <math.h>
#include <stdlib.h>

#include <servo.h>

#define UART_ID uart0
#define BAUD_RATE 115200

const int tx = 16;
const int rx = 17;
const int servoPinX = 19;
const int servoPinY = 18;

QueueHandle_t xQueueAdc;

typedef struct adc {
    int axis;
    int val;
} adc_t;

void servo_task(void *p) {
    adc_t data;
  
    uart_init(UART_ID, BAUD_RATE);
 
    gpio_set_function(tx, GPIO_FUNC_UART);
    gpio_set_function(rx, GPIO_FUNC_UART); 
    
    setServo(servoPinX, 1400);
    setServo(servoPinY, 1400);

    while (1) {
        if(xQueueReceive(xQueueAdc, &data, portMAX_DELAY)){
            // printf("Eixo: %d\n Posicao: %d\n", data.axis, data.val);

            int millis = 400 + (data.val * 2000 / 4095);
            printf("Millis: %d\n", millis);

            if (data.axis == 0) {
                setMillis(servoPinX, millis);
            } 
            else if (data.axis == 1) {
                setMillis(servoPinY, millis);
            }

            // uint8_t mais_sig = (data.val >> 8) & 0xFF;
            // uint8_t menos_sig = data.val & 0xFF;
            // uint8_t eop = 0xFF;

            // uart_putc_raw(UART_ID, data.axis);
            // uart_putc_raw(UART_ID, mais_sig);
            // uart_putc_raw(UART_ID, menos_sig);
            // uart_putc_raw(UART_ID, eop);

            // vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

void x_task(void *p){
    adc_init();
    adc_gpio_init(27);

    adc_t data;
    while (1) {
        int result;
        adc_select_input(0);
        result = adc_read();
        // printf("Valor em x: %d\n", result);
        // result = result - (4095/2);
        // result = result/8;
        // if (result > -30 && result < 30){
            // result = 0;
        // }
        // printf("Posicao X: %d\n", result);
        data.axis = 0;
        data.val = result;
        xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void y_task(void *p){
    adc_init();
    adc_gpio_init(26);

    adc_t data;
    while (1) {
        int result;
        adc_select_input(1);
        result = adc_read();
        // printf("Valor em y: %d\n", result);
        // result = result - (4095/2);
        // result = result/8;
        // if (result > -30 && result < 30){
            // result = 0;
        // }
        // printf("Posicao Y: %d\n", result);
        data.axis = 1;
        data.val = result;
        xQueueSend(xQueueAdc, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main() {
    stdio_init_all();

    xQueueAdc = xQueueCreate(32, sizeof(adc_t));

    xTaskCreate(servo_task, "servo_task", 4096, NULL, 1, NULL);
    xTaskCreate(x_task, "x_task", 4096, NULL, 1, NULL);
    xTaskCreate(y_task, "y_task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
