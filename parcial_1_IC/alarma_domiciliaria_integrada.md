# Sistema de alarma domiciliaria

## Enunciado

Utilizando `GPIO`, interrupciones por puerto, `SysTick`, `Timer0` y `ADC`, realizar un sistema de alarma domiciliaria con una `LPC1769`.

El sistema deberá iniciarse en estado `DESACTIVADO`. Cuando se presione un pulsador conectado a `P0.10`, mediante interrupción por puerto, pasará al estado `SALIDA`, durante el cual dispondrá de `15 segundos` para abandonar el lugar. Ese tiempo deberá controlarse con `SysTick`, haciendo titilar un LED en `P1.8` cada `500 ms`.

Finalizado ese tiempo, el sistema pasará a estado `ACTIVADO`. En ese estado, una interrupción por flanco ascendente en `EINT2` indicará apertura de puerta o ventana y hará pasar el sistema a estado `ENTRADA`, otorgando `10 segundos` para desactivar la alarma.

La desactivación deberá realizarse leyendo una tensión analógica en `AD0.1`. Si el valor convertido supera un umbral prefijado antes de que termine el tiempo de entrada, el sistema volverá a `DESACTIVADO`. Si no ocurre, se pasará a estado `ALARMA`, activando una sirena en `P1.9`.

Mientras el sistema esté en `ALARMA`, un `Timer0` deberá generar una señal intermitente para la sirena, alternando `1 segundo` encendida y `1 segundo` apagada. Solo se podrá salir de ese estado si la entrada analógica vuelve a superar el umbral de desactivación.

Se pide:

- El programa completo en `C`.
- Configuración de `GPIO`, `ADC`, `SysTick`, `Timer0`, `NVIC`, interrupciones por puerto y externas.
- Diagrama de estados.
- Cálculos de tiempos y justificación de umbrales.
