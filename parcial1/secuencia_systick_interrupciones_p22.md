# Variantes de secuencia temporizada con SysTick e interrupciones

## Enunciado

Utilizando `SysTick`, realizar un programa que implemente una secuencia temporizada ante interrupciones externas. En los parciales relevados aparece este mismo ejercicio con las siguientes variantes:

### Variante 1

- Cada vez que se produzca una interrupción por flanco ascendente en `P2.2`, sacar por el pin `P2.1` la secuencia mostrada.
- Si se produce una interrupción por flanco descendente en `P2.2` mientras se está ejecutando la secuencia, se debe poner en `1` la salida `P2.1` y dar por finalizada la secuencia.
- La entrada debe configurarse con resistencia de `pull-down`.
- Suponer `cclk = 40 MHz`.

### Variante 2

- Cada vez que se produzca una interrupción por flanco descendente en `EINT2`, sacar por el pin `P2.4` la secuencia mostrada.
- Si se produce una nueva interrupción por `EINT2` mientras se está ejecutando la secuencia, se debe poner en `1` la salida `P2.4` y dar por finalizada la secuencia.
- El programa no debe hacer uso de retardos por software.
- Enmascarar los pines del puerto `2` que no se utilicen.
- Suponer `cclk = 60 MHz`.
- Se pide el programa completo debidamente comentado y los respectivos cálculos de tiempo asignados al `SysTick`.

### Variante 3

- Cada vez que se produzca una interrupción por nivel alto en `EINT1`, sacar por los pines `P2.0` y `P2.1` la secuencia mostrada.
- Si se produce una nueva interrupción por `EINT1` mientras se está ejecutando la secuencia, esta debe reiniciarse desde el comienzo, descartando la que se venía generando.
- Enmascarar los pines del puerto `2` que no se utilicen.
- Suponer `cclk = 20 MHz`.

## Nota

En el material recibido no se incluyó la figura o forma de onda correspondiente a la "secuencia mostrada".
