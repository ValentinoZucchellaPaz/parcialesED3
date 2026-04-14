# Contador de personas para una sala

## Enunciado

Utilizando `Timer1`, `GPIO`, `NVIC` e interrupciones externas, realizar un sistema contador de personas para una sala.

El ingreso de una persona será detectado mediante una interrupción por flanco ascendente en `EINT1`, mientras que el egreso se detectará mediante una interrupción por flanco ascendente en `EINT2`. Cada evento deberá modificar un contador que no podrá ser menor que `0`.

El valor del contador deberá mostrarse en binario sobre los pines `P2.0` a `P2.3`. Además, un `Timer1` deberá generar una interrupción cada `1 segundo` para actualizar un LED en `P1.5`, el cual deberá titilar mientras haya al menos una persona dentro de la sala, y permanecer apagado cuando el contador valga `0`.

Si la cantidad de personas alcanza el valor `10`, se deberá encender un LED de capacidad máxima en `P1.6`. Mientras este LED se encuentre encendido, no deberán contarse nuevos ingresos, aunque sí deberán registrarse los egresos.

Un pulsador conectado al pin `P0.4`, con interrupción por puerto, deberá reiniciar el contador a `0` en cualquier momento.

Se pide:

- El programa completo en `C`.
- Configuración de `GPIO`, `NVIC`, `Timer1`, interrupciones por puerto y externas.
- Explicación del criterio de actualización del contador.
- Cálculos del temporizador.
