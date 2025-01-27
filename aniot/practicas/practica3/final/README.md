---
title: Práctica 3. Programación con tareas y eventos en ESP-IDF
author: Pablo C. Alcalde y Alejandro de Celis
---
# Funcionamiento

## Finite State Machine

Estableceremos una máquina de estado finito para gestionar los diferentes modos de operación requeridos en nuestro programa.

Tendrá los siguiente modos:
- Monitorización de Sensores
 + En este modo, por medio de eventos señalizaremos la existencia de nuevas mediciones en nuestro sensor y las enviaremos o almacenaremos dependiendo de nuestro estado de conexión.
- Consola
 + En este modo, nuestro funcionamiento será un simple bucle que en función al comando introducido por input nos devuelva el comportamiento deseado.

Para poder gestionar los cambios de estado utilizaremos un event loop donde enviaremos `STATE_EVENT` correspondiente al estado deseado.
Cuando se reciba el evento correpondiente, un callback se encargará de activar y desactivar los procesos correspondientes a cada uno de los modos de operación.

## Monitoring

## Console

