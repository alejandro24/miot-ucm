---
title: Práctica 6. Modos de bajo consumo.
author: Pablo C. Alcalde & Alejandro de Celis
---

# Tarea 1
> Compila y ejecuta el ejemplo `nvs_rw_value` proporcionado por ESP-IDF. Observa la salida. Amplíalo para que, además del número de reinicios, se escriba otro par clave-valor en NVS, en este caso almacenando una cadena. Tras cada reinicio, lee el valor de dicha cadena y muéstralo por pantalla. Es recomendable que tengas a mano la API para la escritura/lectura de pares. En esa misma sección encontrarás ejemplos de uso de las funciones que necesitarás para escribir/leer cadenas.

Se adjunta un [programa](./nvs_rw_value/main/nvs_value_example_main.c) que guarda en una cadena, junto con el número de reinicios una cadena de caracteres donde se guarda este número de una manera descriptiva.

# Tarea 2
> Hacer funcionar el ejmplo, permitiendo que volvamos de light-sleep únicamente por un _timer_ o por _GPIO_.


## Cuestión
> ¿Qué número de GPIO está configurado por defecto para despertar al sistema? ¿Está conectado dicho GPIO a algún elemento de la placa ESP Devkit-c que estamos usando? Puedes tratar de responder consultando el esquemático de la placa

Está configurado de serie para usar el `GPIO 0`.

> ¿Qué flanco provocará que salgamos de light-sleep tras configurar el GPIO con gpio_wakeup_enable(GPIO_WAKEUP_NUM, GPIO_WAKEUP_LEVEL == 0 ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL)?

Según los esquemáticos de la placa, al pulsar el botón se abre el paso de corriente que nos conecta con *GND* lo que significa que será un `GPIO_INTR_LOW_LEVEL`.

# Tareas
> Incluir un timer en el código. La aplicación arrancará, configurará un timer para que se ejecute su callback cada 0.5 segundos, y se dormirá durante 3 segundos (con vTaskDelay()). Tras despertar del delay, pasará a light-sleep (configuraremos el mecanismo de despertar para que lo haga en 5 segundos, si no usamos el GPIO correspondiente). El callback del timer simplemente imprimirá un mensaje que incluirá el valor devuelto por `esp_timer_get_time()`.
>
> Fragmento de código incluido en el fichero timer_wakeup.c.

## Cuestión
> ¿Qué observas en la ejecución de los timer?¿Se ejecutan en el instante adecuado? ¿Se pierde alguno?

Se ejecutan todos juntos al volver del _light-sleep_, salvo uno que a veces entra antes de entrar a dormir y todos tiene por lo tanto casi el mismo segundo (ya que no se ejecutan cuando debieron si no que se tratan todos los eventos en cola al volver del sueño.

# Tareas
> Modifica el código anterior para que, tras 5 pasos por _light-sleep_, pasemos a _deep-sleep_. Incluye código para determinar el motivo por el que hemos despertado de deep-sleep y muestralo por pantalla.
>Hemos incluido el fragmento de código necesario 
>
>El siguiente fragmento de código se ha añadido en el fichero: light_sleep_example_main.c.
> <img width="341" alt="image" src="https://github.com/user-attachments/assets/f0402fb9-09d4-428f-ae78-87c94ebd2288" />


## Cuestión

> ¿Qué diferencia se observa al volver de deep-sleep respecto a volver de light-sleep?

Una vez entra en _deep-sleep_ el GPIO lo despierta automáticamente, puesto que al ser el trigger que el pin baje a 0 al dormir no mantiene el voltaje.
Para corregir este comportamiento añadimos la siguiente linea de código
```c
ESP_ERROR_CHECK(rtc_gpio_hold_en(GPIO_WAKEUP_NUM));
```
Investigando sobre como conseguir mantener el estado del digital gpio llegamos a este [post](https://electronics.stackexchange.com/questions/350158/esp32-how-to-keep-a-pin-high-during-deep-sleep-rtc-gpio-pull-ups-are-too-weak) el cual nos señala tambien que deberíamos usar `gpio_deep_sleep_hold_en`, sin embargo en nuestro ejemplo no fué necesario.
# Tareas

Completar la aplicación de la [practica 3](./../practica3/final/README.md) de modo que:

Se configure el gestor de energía para que entre automáticamente en light-sleep cuando sea posible.
Tras 12 horas de funcionamiento, pasará al modo deep-sleep durante otras 12 horas (para pruebas, en lugar de 12 horas probadlo con 1 minuto).
Compruebe el motivo por el que se produce cada reinicio y lo anote en NVS.
Escriba en NVS la última medida del sensor tomada.

Por limitaciones de tiempo, hemos decidido implementar directamente esta capability en el trabajo final y conjunto con RPI-I y RPI-II. 

El código que se ha desarrollado e incluido es el siguiente (incluido en el directorio powemanager incluido con esta entrega):

power_manager.h

#ifndef POWER_MANAGER_H_
#define POWER_MANAGER_H_

#include "esp_event.h"
#include <time.h>

ESP_EVENT_DECLARE_BASE(POWER_MANAGER_EVENT);

#define POWER_MANAGER_DEEP_SLEEP_EVENT 0

// Configuración del rango horario activo por defecto
#define DEFAULT_START_HOUR 8
#define DEFAULT_END_HOUR 22

// Duración por defecto si no hay RTC (14 horas activo, 10 en Deep Sleep)
#define DEFAULT_ACTIVE_HOURS 14
#define DEFAULT_SLEEP_HOURS 10

// 1 hora = 36 * 100.000.000 microsegundos
#define CONVERSION_HOURS_TO_MICROSECONDS (36ULL * 100 * 1000 * 1000)
// 1 minuto = 60 * 1.000.000 microsegundos
#define CONVERSION_MINUTES_TO_MICROSECONDS (60L * 1000 * 1000)

void power_manager_init();

esp_err_t power_manager_set_sntp_time(struct tm *timeinfo);

void power_manager_enter_deep_sleep();

void power_manager_deinit();

#endif /* POWER_MANAGER_H_ */


power_manager.c

#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <esp_event.h>
#include <esp_sleep.h>
#include <freertos/FreeRTOS.h>
#include <esp_system.h>
#include <esp_err.h>
#include <esp_check.h>

#include "power_manager.h"

static const char *TAG = "POWER_MANAGER";

ESP_EVENT_DEFINE_BASE(POWER_MANAGER_EVENT);

static esp_timer_handle_t deep_sleep_timer;

static int64_t start_time = 0;

static void deep_sleep_timer_callback(void *arg)
{
    // Obtener el timestamp final cuando el temporizador se dispara
    int64_t end_time = esp_timer_get_time();

    // Calcular el tiempo transcurrido (en microsegundos)
    int64_t elapsed_time = end_time - start_time;

    // Mostrar el tiempo transcurrido en segundos
    ESP_LOGI(TAG, "Tiempo de ejecución del temporizador: %lld microsegundos", elapsed_time);

    esp_event_post(POWER_MANAGER_EVENT, POWER_MANAGER_DEEP_SLEEP_EVENT, NULL, 0, portMAX_DELAY);
}

void power_manager_enter_deep_sleep()
{
    ESP_LOGI(TAG, "Entrando en deep_sleep.");
    esp_deep_sleep_start();
}

void power_manager_init()
{
// Configurar gestor energia para entrar automaticamente en light_sleep
#if CONFIG_PM_ENABLE
    // Configure dynamic frequency scaling:
    // maximum and minimum frequencies are set in sdkconfig,
    // automatic light sleep is enabled if tickless idle support is enabled.
    esp_pm_config_t pm_config = {
        .max_freq_mhz = 160, // ESP32c3: 160 MHz, ESP32-devkit-c: 240 MHz
        .min_freq_mhz = 80,  // 80 MHz
#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
        .light_sleep_enable = true
#endif
    };
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
    ESP_LOGI(TAG, "Configurado gestor de energia automatico");
#endif // CONFIG_PM_ENABLE

    uint64_t sleep_hours = DEFAULT_SLEEP_HOURS * CONVERSION_HOURS_TO_MICROSECONDS;
    uint64_t active_hours = DEFAULT_ACTIVE_HOURS * CONVERSION_HOURS_TO_MICROSECONDS;

    /* Configurar timer para despertar de deep_sleep cada 10 horas */
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(sleep_hours));
    ESP_LOGI(TAG, "Configurado wakeup por timer en %d horas", (DEFAULT_SLEEP_HOURS));

    /* Configurar timer para entrar en modo deep_sleep cada 14 horas */
    esp_timer_create_args_t deep_sleep_timer_args = {
        .callback = deep_sleep_timer_callback,
        .name = "deep_sleep_timer"};

    ESP_ERROR_CHECK(esp_timer_create(&deep_sleep_timer_args, &deep_sleep_timer));
    ESP_ERROR_CHECK(esp_timer_start_once(deep_sleep_timer, active_hours));
    ESP_LOGI(TAG, "Configurado deep_sleep en %d horas", (DEFAULT_ACTIVE_HOURS));
}

esp_err_t power_manager_set_sntp_time(struct tm *timeinfo)
{
    esp_err_t errcode = ESP_OK;

    int64_t wakeup_time_in_minutes = 0;     // tiempo durmiendo hasta que salte el wakeup
    int64_t time_till_sleep_in_minutes = 0; // tiempo restante hasta acabar el rango activo

    bool enter_deep_sleep_now = false;

    /* --- Tenemos hora SNTP, deshabilitar anteriores timers y wakeup source ---
     */
    if (esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER) != ESP_OK) // Try to deactivate timer trigger only
        errcode = esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);   // disable all

    ESP_RETURN_ON_ERROR(errcode, TAG, "Error desactivando el timer de wakeup por defecto");

    errcode = esp_timer_stop(deep_sleep_timer);
    ESP_RETURN_ON_ERROR(errcode, TAG, "Error parando el timer de entrar en deep_sleep por defecto");

    /* --- Habilitar con los nuevos valores calculados usando la hora de SNTP ---
     */
    // Obtener las horas de inicio y fin desde las configuraciones de Kconfig
    const char *start_time_str = CONFIG_PM_ACTIVE_START_HOUR;
    const char *end_time_str = CONFIG_PM_ACTIVE_END_HOUR;

    // Convertir las cadenas de tiempo (HH:MM) a horas y minutos
    int start_hour, start_minute;
    int end_hour, end_minute;

    // Parsear la hora de inicio
    sscanf(start_time_str, "%2d:%2d", &start_hour, &start_minute);
    // Parsear la hora de fin
    sscanf(end_time_str, "%2d:%2d", &end_hour, &end_minute);

    // Modulo con 24 para evitar las 24h y quedarnos con 00h
    start_hour = start_hour % 24;
    end_hour = end_hour % 24;

    // Convertir la hora actual a minutos del día para hacer la comparación más fácil
    int64_t current_time_in_minutes = timeinfo->tm_hour * 60 + timeinfo->tm_min;
    int64_t start_time_in_minutes = start_hour * 60 + start_minute;
    int64_t end_time_in_minutes = end_hour * 60 + end_minute;

    // Contador para el tiempo total en rango activo
    int64_t total_active_time_in_minutes = 0;

    ESP_LOGI(TAG, "Configurando power manager con rango activo: %s - %s.",
             start_time_str, end_time_str);

    // Rango activo cruzado, p.ej desde las 22:00 hasta las 08:00
    if (start_time_in_minutes > end_time_in_minutes)
    {
        total_active_time_in_minutes =
            (24 * 60) - start_time_in_minutes + end_time_in_minutes;

        if (current_time_in_minutes >= start_time_in_minutes ||
            current_time_in_minutes < end_time_in_minutes)
        {
            // Estamos en rango activo, configurar timer para que nos avise llegada la
            // hora de dormir

            // Hora actual menor que hora de fin, estamos en el dia
            if (current_time_in_minutes < end_time_in_minutes)
                time_till_sleep_in_minutes = end_time_in_minutes - current_time_in_minutes;
            else // Hora actual mayor que hora de fin, estamos en el dia anterior
                time_till_sleep_in_minutes = (24 * 60) - current_time_in_minutes + end_time_in_minutes;

            // Calculamos el tiempo del timer de wakeup
            // (minutos totales del dia - minutos_rango_activo)
            wakeup_time_in_minutes = (24 * 60) - total_active_time_in_minutes;
        }
        else
        {
            // Estamos fuera del rango activo, entrar forzadamente en deep_sleep
            // Calcular el tiempo hasta el inicio del rango horario siguiente

            // Hora actual menor que hora de fin, estamos en el dia
            if (current_time_in_minutes < start_time_in_minutes)
                wakeup_time_in_minutes = start_time_in_minutes - current_time_in_minutes;
            else // Hora actual mayor que hora de fin, estamos en el dia anterior
                wakeup_time_in_minutes = (24 * 60) - current_time_in_minutes + start_time_in_minutes;

            enter_deep_sleep_now = true;
        }
    }
    else // Rango activo normal, p.ej desde las 08:00 hasta las 22:00
    {
        total_active_time_in_minutes = end_time_in_minutes - start_time_in_minutes;

        if (current_time_in_minutes >= start_time_in_minutes &&
            current_time_in_minutes < end_time_in_minutes)
        {
            // Estamos en rango activo, configurar timer para que nos avise cuando sea
            // la hora de dormir
            time_till_sleep_in_minutes = end_time_in_minutes - current_time_in_minutes;

            // Calculamos el tiempo del timer de wakeup
            //  (minutos totales del dia - minutos_rango_activo)
            wakeup_time_in_minutes = (24 * 60) - total_active_time_in_minutes;
        }
        else
        {
            // Estamos fuera del rango activo, entrar forzadamente en deep_sleep
            // Calcular el tiempo hasta el inicio del rango horario siguiente
            wakeup_time_in_minutes = start_time_in_minutes - current_time_in_minutes;

            if (wakeup_time_in_minutes < 0)
            {
                // Ajustar si es negativo (es decir, si la hora actual es mayor que la hora de inicio)
                wakeup_time_in_minutes += 24 * 60;
            }

            enter_deep_sleep_now = true;
        }
    }

    uint64_t wkup_time_us = (uint64_t)(wakeup_time_in_minutes * CONVERSION_MINUTES_TO_MICROSECONDS);
    uint64_t time_till_sleep_us = (uint64_t)(time_till_sleep_in_minutes * CONVERSION_MINUTES_TO_MICROSECONDS);

    ESP_LOGI(TAG, "Configurado wakeup tras deep_sleep en %" PRId64 " horas y %" PRId64 " minutos (%" PRId64 " minutos, %" PRIu64 " us).",
             wakeup_time_in_minutes / 60, wakeup_time_in_minutes % 60, wakeup_time_in_minutes, wkup_time_us);

    errcode = esp_sleep_enable_timer_wakeup(wkup_time_us); // En microsegundos
    ESP_RETURN_ON_ERROR(errcode, TAG, "Error activando el nuevo timer de wakeup por rango horario");

    start_time = esp_timer_get_time(); // Tiempo antes de iniciar el timer de deep_sleep, algo funciona mal y no se que es, vamos a comprobarlo

    if (enter_deep_sleep_now)
        power_manager_enter_deep_sleep();
    else
    { // Habilitar timer para que nos avise cuando sea hora de entrar en deep_sleep
        errcode = esp_timer_start_once(deep_sleep_timer, time_till_sleep_us);
        ESP_RETURN_ON_ERROR(errcode, TAG, "Error activando el nuevo timer de entrar en deep_sleep por rango horario");

        ESP_LOGI(TAG, "Tiempo hasta entrar en deep_sleep: %" PRId64 " horas y %" PRId64 " minutos (%" PRId64 " minutos, %" PRIu64 " us).",
                 time_till_sleep_in_minutes / 60, time_till_sleep_in_minutes % 60, time_till_sleep_in_minutes, time_till_sleep_us);
    }

    return errcode;
}


