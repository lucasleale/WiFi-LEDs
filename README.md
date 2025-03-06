# WiFi-LEDs
Para fines de archivo:
Desarrollo electrónico y de firmware utilizando C/C++ con framework Arduino con microcontroladores ESP32 aplicados al IoT.

El objetivo del proyecto fue fabricar 3 LEDs de alta potencia que se comuniquen entre sí de forma inalámbrica para que hagan un juego de luces específico y sincronizado. Además se solicitó el uso de un sensor PIR para desactivar las luces cuando no haya público y así ahorrar energía.

El proyecto constó de 4 microcontroladores ESP32 conectadods entre sí a traves de UDP mediante una red local. 3 de los 4 ESP32 están conectados a cada LED, estos se van intercambiando una variable que consta de un contador y un loop. La variable controla la intensidad de cada LED según corresponda. 
El cuarto ESP32 se conecta a un sensor PIR en una ubicación estratégica para detectar el ingreso del público, si en la sala pasa determinado tiempo sin público, este ESP32 envía una señal de forma inalámbrica a cada ESP32/LED para apagarse.
