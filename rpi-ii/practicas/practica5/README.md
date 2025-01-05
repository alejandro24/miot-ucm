---
title: Práctica 5. El protocolo MQTT
author: Pablo C. Alcalde, Alejandro de Celis
...

# Tarea 1
> Observa la ayuda de ambas ordenes, utilizando el argumento --help. Identifica los parámetros que te permitirán especificar el broker destino, el topic a utilizar y, en el caso de la publicación, el mensaje a enviar.

- `-h <broker>` selecciona el broker.
- `-t <topic>` selecciona el topic.
- `-m <message>` adjunta el mensaje.

# Tarea 2
> Suscribámonos al topic # en el broker, utilizando para ello la orden:
> `mosquitto_sub -h test.mosquitto.org  -t "#"`
> Pausa la salida en cuanto puedas. ¿A qué corresponden los mensajes que estás obteniendo?

# Tarea 3
> Suscríbete al topic /MIOT/tunombre y observa si recibes los resultados tras la publicación correspondiente. ¿Cómo podrías suscribirte a todos los mensajes publicados por compañeros?

El comando siguiente `moquitto_sub -h <server> -t /MIOT/#`

# Tarea Entregable 1
> Realiza un análisis del intercambio de mensajes necesario para un proceso de publicación/suscripción contra el broker de test. Incide en el tipo de protocolo de capa de transporte que utiliza MQTT, mensajes de datos y control, sobrecarga del protocolo de capa de aplicación, y en general, cualquier aspecto que consideres de interés, incluyendo el uso de opciones relativas a QoS.

Como se puede ver en esta [captura](./subscripcion.pcapng) en un primer lugar se realiza el handshake TCP y a continuación:
+ Un *MQTT Connect* packet a modo de solicitud, en el cual se incluyen las credenciales correspondientes a la conexión. En este caso, como protegimos nuestro servidor con usuario y contraseña, se pueden observar un `0xc2` *User Name Flag* que se acompaña de un *User Name* y un *Password* acompañadas por las longitudes de ambas cadenas de caracteres.

+ *MQTT Connect Ack* el mensaje de conexión se responde con un Acknowledgement que a su vez ha de ser confirmado por la capa de transporte.

+ *MQTT Subscribe Request* el cliente envia un mensaje para suscribirse al topic donde acompaña con el valor de QoS que señala la certeza a la hora de recepcion recepción de mensajes en el topic. Es importante señalar que es el servidor el que tendrá que encargarse de llegar una caché donde guardar estos datos que aún no han sido recibidos en caso de que algún motivo no pudiese recibir confirmación de recepción por parte del cliente.
  - `QoS = 0` como máximo una vez.
  - `QoS = 1` al menos una vez.
  - `QoS = 2` exactamente una vez.

+ *MQTT Subscribe Ack* para confirmar la recepción y registro de la subscripción por por parte del servidor.

# Tarea Entregable 2

> Cada alumno propondrá una solución para monitorizar un edificio inteligente a través de un sistema de mensajería MQTT. Para ello, cabe destacar que el edificio constará de:
>
> - Un identificador del tipo EDIFICIO_TUPUESTODELABORATORIO.
> - Un conjunto de plantas, identificadas por la cadena "P_NUMPLANTA".
> - En cada planta, cuatro alas (norte -N-, sur -S-, este -E-, oeste -O-)
> - En cada ala, un conjunto de salas, identificadas por un valor numérico.
> - En cada sala, cuatro sensores: TEMP (temperatura), HUM (humedad), LUX (luminosidad), VIBR (vibración).
>
> Se pide, en primer lugar, diseñar la jerarquía de topics que permita una correcta monitorización de los edificios.
>
> En segundo lugar, se desarrollará un programa Python cliente que publique, periódicamente y de forma aleatoria, objetos JSON (opcionalmente puedes utilizar CBOR, usando los módulos correspondientes) que incluyan el valor de temperatura, humedad, luminosidad o vibración para una determinada sala del edificio, elegida también aleatoriamente, a través del topic correspondiente. Estos mensajes estarán espaciados en el tiempo un número aleatorio de segundos.
>
> En tercer lugar, se piden las wildcards que permitan consultar distintos tipos de información jerárquica. Por ejemplo:
>
> - Todos los mensajes de temperatura para el edificio.
> - Todos los mensajes de vibración del ala oeste de la planta 2 del edificio.
> - Todos los mensajes de sensorización de la sala 4 del ala Sur de la planta 7 del edificio.
> ...
> En último lugar, se pide desarrollar un programa Python que actúe a modo de alarma, y que muestre mensajes sólo si algún valor recibido para los datos sensorizados supera un umbral preestablecido. En dicho caso, el programa mostrará el edificio, planta, ala, sala y sensor que ha producido la alarma, junto con su valor numérico.
>
> Puedes utilizar el módulo JSON para parsear los objetos recibidos en Python.

Se resuelve lo requerido en el siguiente código por un lado el [cliente](./mqtt_subscriber.py) y por otro el [servidor](./mqtt_basic.py) adjuntando la siguiente [captura](./funcionamiento_edificio.pcapng) de tráfico mostrando su funcionamiento.

# Tarea Entregable 3
> Modifica el ejemplo proporcionado para que se integre en tu entorno de monitorización de un edificio. Así, el firmware procederá creando una tarea que, periódicamente (cada interval segundos), publique un valor aleatorio para los cuatro parámetros monitorizados.
>
> Además, deberás diseña un sistema basado en MQTT mediante el cual puedas controlar, externamente, el comportamiento del sensor, atendiendo a los siguientes criterios:
>
> 1. El tiempo (interval) mediante que transcurrirá entre publicaciones será configurable a través de un proceso de publicación desde tu terminal Linux y suscripción del ESP32 a un topic determinado.
> 2. La sensorización (y publicación de datos) podrá activarse o desactivarse bajo demanda a través de la publicación desde tu terminal Linux y suscripción del ESP32 a un topic determinado.
>
> Por ejemplo, imagina que tu sensor publica mensajes de sensorización en el topic /EDIFICIO_3/P_4/N/12/(TEMP|HUM|LUX|VIBR). Para controlar el intervalo de publicación de datos desde dicho ESP32 y fijarlo a 1 segundo, podríamos publicar un mensaje utilizando la orden:
>
> `mosquitto_pub -t /EDIFICIO_3/P_4/N/12/interval -m "1000" -h IP_BROKER`
>
> Para desactivar el sensor, podríamos utilizar:
>
> `mosquitto_pub -t /EDIFICIO_3/P_4/N/12/disable -m "" -h IP_BROKER`
>
> Para activar el sensor, podríamos utilizar:
>
> `mosquitto_pub -t /EDIFICIO_3/P_4/N/12/enable -m "" -h IP_BROKER`
>
> + Opcionalmente, puedes ampliar tu solución para que cada sensor se active o desactive individualmente bajo demanda. En este caso, elige y documenta el topic utilizado.

Se adjunta el siguiente [proyecto](./tcp/README.md).
