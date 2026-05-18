# Control de velocidad de un ventilador

## Enunciado

Utilizando `ADC`, `Timer0`, `SysTick` y `GPIO`, diseñar un sistema de control de velocidad de un ventilador.

La velocidad deseada será ingresada mediante un potenciómetro conectado al canal `AD0.5`. El sistema deberá muestrear la entrada analógica cada `50 ms` usando `SysTick` y clasificar el valor leído en cuatro niveles de velocidad.

Cada nivel deberá representarse encendiendo una combinación distinta de LEDs conectados a `P1.0`, `P1.1`, `P1.2` y `P1.3`. Además, el sistema deberá generar en `P2.7` una señal cuadrada cuya frecuencia dependa del nivel seleccionado. Esta señal deberá obtenerse mediante interrupciones periódicas de `Timer0`.

Un pulsador conectado a `EINT0` permitirá habilitar o deshabilitar la salida del ventilador. Cuando la salida esté deshabilitada, la señal en `P2.7` deberá permanecer en `0`, independientemente del valor leído por el `ADC`.

Si el nivel máximo se mantiene durante más de `5 segundos`, deberá encenderse un LED de sobreexigencia en `P1.4`, el cual se apagará únicamente si durante `3 segundos` consecutivos la velocidad vuelve a un nivel inferior.

Se pide:

- El programa completo en `C`.
- Configuración de `ADC`, `GPIO`, `Timer0`, `SysTick`, `NVIC` y `EINT0`.
- Cálculo de los registros necesarios para cada frecuencia.
- Explicación del criterio de temporización.
