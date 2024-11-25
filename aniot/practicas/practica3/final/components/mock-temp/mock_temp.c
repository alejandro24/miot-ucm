#include "mock_temp.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_timer.h"
#include "portmacro.h"

static const char *TAG = "MOCK_TEMP";

esp_event_loop_handle_t temp_loop_connect;

ESP_EVENT_DEFINE_BASE(TEMP_MOCK);

#define MEASUREMENT_DELAY CONFIG_MEASUREMENT_DELAY

esp_timer_handle_t measurement_timer;

uint64_t us_measurement_delay = MEASUREMENT_DELAY * 1000000u;

float curr_temp;

static void measurement_timer_callback(void * args);

void temp_mock_init(esp_event_loop_handle_t event_loop)
{
    // Sensor logic goes here.
    temp_loop_connect = event_loop;

    const esp_timer_create_args_t measurement_timer_args = {
        .callback = &measurement_timer_callback,
        .name = "measurement"
    };
    esp_timer_create(&measurement_timer_args, &measurement_timer);

    ESP_LOGI(TAG, "Measurement Initialized");
}

esp_err_t temp_mock_start()
{
    return esp_timer_start_periodic(measurement_timer, us_measurement_delay);
}

esp_err_t temp_mock_pause()
{
    esp_err_t ret = esp_timer_stop(measurement_timer);
    if (ret == ESP_OK) {
        esp_event_post_to(temp_loop_connect, TEMP_MOCK, TEMP_MOCK_PAUSED, NULL, 0, 0);
    }
    return ret;
}

esp_err_t temp_mock_restart()
{
    return esp_timer_restart(measurement_timer, us_measurement_delay);
}

static void measurement_timer_callback(void * arg)
{
    curr_temp += 1.0f;
    ESP_LOGI(TAG, "Temp got new data");
    esp_event_post_to(temp_loop_connect, TEMP_MOCK, TEMP_MOCK_NEW_MEASUREMENT, &curr_temp , sizeof(float), portMAX_DELAY);
}

