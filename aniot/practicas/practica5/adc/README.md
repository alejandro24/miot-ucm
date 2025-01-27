---
title: Práctica 5. Uso de ADC.
author: Pablo C. Alcalde & Alejandro de Celis
...
# Voltaje a distancia
**Una vez conseguido el voltaje que está devolviendo el sensor, ¿qué expresión usarás en el código para obtener la distancia en centímetros?**

Utilizamos la siguiente 


<img width="534" alt="image" src="https://github.com/user-attachments/assets/da71d398-4c25-4b33-b7df-f5e93a5c7780" />



# Requisitos
> Usaremos el sensor de distancia GP2Y0A41SK0F de Sharp conectado al ADC de ESP32. Deberás conectar la alimentación del senor al pin de 5V del ESP32, las tierras en común y el cable de medida a un pin GPIO del ESP32 que configurarás para usar un canal de ADC.
>
> Muestrear el ADC correspondiente cada segundo, haciendo la media de N lecturas en cada muestreo (siendo N una constante que se puede modificar via menuconfig). Usad un timer para el muestreo. Se notificará mediante *un evento^, la disponibilidad de un nuevo dato. El código relativo al acceso al sensor estará en un componente separado con llamadas para la configuración, arranque/parada de las medidas, y obtener el último valor de distancia medido.
> El programa principal registará un handle del evento correspondiente. En dicho handle se invocará a la función del módulo anterior para conseguir el valor de la última distancia medida, y se mostrará por pantalla.
> Se deberá comprobar la salida de las funciones invocadas, e informar en caso de error. Utiliza las funciones proporcionadas por ESP-IDF documentadas en su web

Se adjunta el código encargado de realizar esas tareas requeridas dentro de este proyecto.
