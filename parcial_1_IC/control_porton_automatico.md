# Control de portón automático

## Enunciado

Utilizando `GPIO`, interrupciones externas, `SysTick` y `Timer0`, diseñar un sistema de control para un portón automático con una `LPC1769`.

El sistema debe permanecer inicialmente en estado `CERRADO`, con un LED rojo encendido en `P1.0`. Cuando se produzca una interrupción por flanco ascendente en `EINT0`, el sistema deberá pasar al estado `ABRIENDO`, apagando el LED rojo y encendiendo un LED verde en `P1.1`.

Durante el estado `ABRIENDO`, un `Timer0` deberá generar un tiempo de apertura de `8 segundos`. Una vez cumplido ese tiempo, el sistema pasará al estado `ABIERTO`, manteniéndose así durante `5 segundos`, tiempo que deberá ser controlado con `SysTick`.

Finalizado ese período, el sistema pasará al estado `CERRANDO`, apagando el LED verde y haciendo titilar ambos LEDs alternadamente cada `250 ms`. Si durante este estado se detecta una nueva interrupción por `EINT0`, el portón deberá volver inmediatamente al estado `ABRIENDO`.

Además, un pulsador conectado al pin `P0.8` deberá actuar como sensor de obstáculo. Si este pin toma valor lógico `1` mientras el portón se encuentra en `CERRANDO`, se deberá reiniciar la apertura completa desde cero.

Se pide:

- El programa completo en `C`.
- Configuración de `GPIO`, `NVIC`, `SysTick`, `Timer0` e interrupciones.
- Diagrama de estados del sistema.
- Cálculos de tiempos claramente justificados.
