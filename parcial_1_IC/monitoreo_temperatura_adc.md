# Monitoreo de temperatura con ADC

## Enunciado

Utilizando `ADC`, `SysTick`, `GPIO` e interrupciones por puerto, realizar un sistema de monitoreo de temperatura con una `LPC1769`.

Una señal analógica conectada al canal `AD0.2` representa la temperatura medida dentro de un horno. El sistema deberá muestrear dicha señal cada `100 ms` utilizando `SysTick`.

Mientras la temperatura convertida sea menor a un umbral de referencia, se encenderá un LED en `P2.0`. Si la temperatura supera dicho umbral, se apagará ese LED y comenzará a titilar un LED de advertencia en `P2.1` cada `500 ms`.

Además, un pulsador conectado al pin `P0.15`, configurado con interrupción por puerto, permitirá alternar entre dos umbrales de temperatura distintos. Cada vez que se presione, el sistema deberá cambiar entre `umbral bajo` y `umbral alto`, indicando el modo actual mediante el estado de un LED en `P2.2`.

Si la temperatura supera el umbral alto durante más de `10 segundos` consecutivos, deberá activarse una alarma sonora en `P2.3`, la cual solo podrá desactivarse cuando la temperatura vuelva a estar por debajo del umbral bajo.

Se pide:

- El programa completo en `C`.
- Configuración del `ADC`, `GPIO`, `SysTick` e interrupción por puerto.
- Definición clara de la lógica de control.
- Cálculos temporales y comentarios explicativos.
