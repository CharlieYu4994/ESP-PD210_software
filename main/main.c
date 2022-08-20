/**
 * Copyright (c) 2022 CharlieYu4994
 * 
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "ssd1316.h"

static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
SemaphoreHandle_t xGuiSemaphore;

void app_main(void)
{
    xTaskCreate(guiTask, "gui", 4096*2, NULL, 0, NULL);
}

static void guiTask(void *pvParameter) {

    (void) pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    ssd1316_init();

    lv_color_t* buf1 = heap_caps_malloc(2048 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    static lv_color_t *buf2 = NULL;
    static lv_disp_draw_buf_t disp_buf;

    uint32_t size_in_px = 4096;

    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = ssd1316_flush;
    disp_drv.rounder_cb = ssd1316_rounder;
    disp_drv.set_px_cb = ssd1316_set_px_cb;
    disp_drv.draw_buf = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000));

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));

        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
       }
    }

    free(buf1);
    vTaskDelete(NULL);
}

static void lv_tick_task(void *arg) {
    (void) arg;

    lv_tick_inc(1);
}
