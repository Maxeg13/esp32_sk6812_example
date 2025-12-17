/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "sdkconfig.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gptimer.h"
#include "esp_timer.h"
#include "sk6812_led.h"

const static ColourState* state1Ptr = new ColourState{20, 20, 20};
const static ColourState* state2Ptr = new ColourState{80, 80, 80};
const static ColourState* state3Ptr = new ColourState{0, 255, 0};

static bool timer_on_alarm_cb(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)
{
    static const ColourState* targetPtr{state1Ptr};

    skc6812_led_push(targetPtr);
    targetPtr = targetPtr->targetPtr;

    return true;
}

extern "C" {
void app_main(void) {
    skc6812_led_Init();

    state1Ptr->initTarget(state2Ptr);
    state2Ptr->initTarget(state3Ptr);
    state3Ptr->initTarget(state1Ptr);

    // timer
    gptimer_handle_t gptimer = NULL;
    gptimer_config_t timer_config = {
            .clk_src = GPTIMER_CLK_SRC_DEFAULT,
            .direction = GPTIMER_COUNT_UP,
            .resolution_hz = 100000,
    };
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

    gptimer_event_callbacks_t cbs = {
            .on_alarm = timer_on_alarm_cb,
    };
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, NULL));

    gptimer_alarm_config_t alarm_config = {
            200000, // period = 2s
            0, // counter will reload with 0 on alarm event
            {true}, // enable auto-reload
    };
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer));



    while (true) {
//        skc6812_led_blue_test();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
}