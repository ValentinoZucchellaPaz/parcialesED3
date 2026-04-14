# Control de cámara frigorífica

## Enunciado

Se solicita diseñar el sistema de control para una cámara frigorífica utilizando una LPC1769 (100 MHz), el cual debe iniciar en un estado de `REPOSO` hasta que una interrupción por flanco de subida en el pin `P2.10` (`EINT0`) indique la apertura de la puerta.

Al ocurrir esto, el sistema pasará a un estado de `TOLERANCIA` activando el `SysTick` (configurado a 10 ms) para cronometrar 15 segundos, durante los cuales se debe monitorear el pin `P0.4` (`GPIO`); si este vuelve a `0` (puerta cerrada) el sistema retorna a `REPOSO`, pero si el tiempo agota, se transiciona a un estado de `PRE_ALARMA` donde se hará parpadear un LED en `P1.10` y se iniciará un nuevo conteo de 10 segundos.

Si la puerta no se cierra en este segundo periodo, el sistema entrará en un estado de `ALARMA_CRITICA` activando una sirena en `P1.11`, condición de la cual solo se podrá salir y regresar a `REPOSO` mediante la lectura de un `1` lógico en el pin de reset `P0.5`.
