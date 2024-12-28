#include <stdbool.h>
#include <stdio.h>
#include "esp_err.h"
#include "freertos/projdefs.h"
#include "nvs_flash.h"
#include "driver/i2c_master.h"
#include "icm42670.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "portmacro.h"
#include "led_strip.h"

#define BLINK_GPIO 2
#define I2C_MASTER_SCL_IO 8       /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 10       /*!< gpio number for I2C master data  */
// Our development board only has one i2c so -1 for auto should work fine
#define I2C_MASTER_NUM I2C_NUM_0  /*!< I2C port number for master dev */
#define RETRY_TIMER 5 /*!< retry timer for i2c handler functions */

const char *TAG = "Practica 4";
icm42670_handle_t icm42670_handle = NULL;
i2c_master_bus_handle_t i2c_bus = NULL;
esp_timer_handle_t measurement_timer_handle = NULL;
icm42670_value_t acc, gyro;
led_strip_handle_t led_strip;


void periodic_measurement_callback(void *args)
{
    ESP_ERROR_CHECK(icm42670_get_acce_value(icm42670_handle, &acc));
    ESP_LOGI(TAG, "ACC => x= %f, y= %f, z= %f", acc.x, acc.y, acc.z);
    if (acc.z < 0)
    {
        ESP_LOGI(TAG, "tarjeta está boca abajo");
        led_strip_set_pixel(led_strip, 0, 16, 0, 0);
        led_strip_refresh(led_strip);
    } else {
        ESP_LOGI(TAG, "tarjeta está boca arriba");
        led_strip_set_pixel(led_strip, 0, 0, 0, 16);
        led_strip_refresh(led_strip);
    }

}

esp_err_t i2c_bus_init(void)
{
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    return i2c_new_master_bus(&bus_config, &i2c_bus);
}

esp_err_t led_strip_init(void)
{
    /// LED strip common configuration
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,  // The GPIO that connected to the LED strip's data line
        .max_leds = 1,                 // The number of LEDs in the strip,
        .led_model = LED_MODEL_WS2812, // LED strip model, it determines the bit timing
        .flags = {
            .invert_out = false, // don't invert the output signal
        }
    };

    /// RMT backend specific configuration
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,    // different clock source can lead to different power consumption
        .resolution_hz = 10 * 1000 * 1000, // RMT counter clock frequency: 10MHz
        .mem_block_symbols = 64,           // the memory size of each RMT channel, in words (4 bytes)
        .flags = {
            .with_dma = false, // DMA feature is available on chips like ESP32-S3/P4
        }
    };

    /// Create the LED strip object
    return led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
}

esp_err_t i2c_sensor_icm42670_init(void)
{
    ESP_RETURN_ON_ERROR(i2c_bus_init(), TAG, "failed to initialize bus");

    ESP_RETURN_ON_ERROR(icm42670_create(i2c_bus, ICM42670_I2C_ADDRESS, &icm42670_handle), TAG, "failed to create icm42670 handle");

    const icm42670_cfg_t imu_cfg = {
        .acce_fs = ACCE_FS_2G,
        .acce_odr = ACCE_ODR_400HZ,
        .gyro_fs = GYRO_FS_2000DPS,
        .gyro_odr = GYRO_ODR_400HZ,
    };

    ESP_RETURN_ON_ERROR(icm42670_config(icm42670_handle, &imu_cfg), TAG, "failed to configure sensor");

    icm42670_acce_set_pwr(icm42670_handle, ACCE_PWR_LOWNOISE);
    vTaskDelay(pdMS_TO_TICKS(50));

    return ESP_OK;
}


void app_main(void)
{

    ESP_ERROR_CHECK(i2c_sensor_icm42670_init());
    ESP_ERROR_CHECK(led_strip_init());

    const esp_timer_create_args_t periodic_measurement_args = {
        .callback = &periodic_measurement_callback,
        .name = "measurement",
    };

    ESP_ERROR_CHECK(esp_timer_create(&periodic_measurement_args, &measurement_timer_handle));

    ESP_ERROR_CHECK(esp_timer_start_periodic(measurement_timer_handle, 3000000));
}
