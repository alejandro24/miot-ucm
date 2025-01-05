---
title: Práctica 6. Modos de bajo consumo.
author: Pablo C. Alcalde. Alejandro de Celis
---

# Tarea 1
> Compila y ejecuta el ejemplo `nvs_rw_value` proporcionado por ESP-IDF. Observa la salida. Amplíalo para que, además del número de reinicios, se escriba otro par clave-valor en NVS, en este caso almacenando una cadena. Tras cada reinicio, lee el valor de dicha cadena y muéstralo por pantalla. Es recomendable que tengas a mano la API para la escritura/lectura de pares. En esa misma sección encontrarás ejemplos de uso de las funciones que necesitarás para escribir/leer cadenas.

Se adjunta un [programa](./nvs_rw_value/main/nvs_value_example_main.c) que guarda en una cadena, junto con el número de reinicios una cadena de caracteres donde se guarda este número de una manera descriptiva.

# Tarea 2
> Hacer funcionar el ejmplo, permitiendo que volvamos de light-sleep únicamente por un _timer_ o por _GPIO_.


## Cuestión
> ¿Qué número de GPIO está configurado por defecto para despertar al sistema? ¿Está conectado dicho GPIO a algún elemento de la placa ESP Devkit-c que estamos usando? Puedes tratar de responder consultando el esquemático de la placa

Está configurado de serie para usar el `GPIO 0` que se 

> ¿Qué flanco provocará que salgamos de light-sleep tras configurar el GPIO con gpio_wakeup_enable(GPIO_WAKEUP_NUM, GPIO_WAKEUP_LEVEL == 0 ? GPIO_INTR_LOW_LEVEL : GPIO_INTR_HIGH_LEVEL)?

Según los esquemáticos de la placa, al pulsar el botón se abre el paso de corriente que nos conecta con *GND* lo que significa que será un `GPIO_INTR_LOW_LEVEL`.

# Tareas
> Incluir un timer en el código. La aplicación arrancará, configurará un timer para que se ejecute su callback cada 0.5 segundos, y se dormirá durante 3 segundos (con vTaskDelay()). Tras despertar del delay, pasará a light-sleep (configuraremos el mecanismo de despertar para que lo haga en 5 segundos, si no usamos el GPIO correspondiente). El callback del timer simplemente imprimirá un mensaje que incluirá el valor devuelto por `esp_timer_get_time()`.

## Cuestión
> ¿Qué observas en la ejecución de los timer?¿Se ejecutan en el instante adecuado? ¿Se pierde alguno?

Se ejecutan todos juntos al volver del _light-sleep_, salvo uno que a veces entra antes de entrar a dormir y todos tiene por lo tanto casi el mismo segundo (ya que no se ejecutan cuando debieron si no que se tratan todos los eventos en cola al volver del sueño.

# Tareas
> Modifica el código anterior para que, tras 5 pasos por _light-sleep_, pasemos a _deep-sleep_. Incluye código para determinar el motivo por el que hemos despertado de deep-sleep y muestralo por pantalla.

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

[TODO]

