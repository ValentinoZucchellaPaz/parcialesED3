# Contador con display de 7 segmentos

## Enunciado

Utilizando `SysTick` e interrupciones externas escribir un código en `C` que cuente indefinidamente de `0` a `9`.

Un pulsador conectado a `EINT0` reiniciará la cuenta a `0` y se mantendrá en ese valor mientras el pulsador se encuentre presionado.

Un pulsador conectado a `EINT1` permitirá detener o continuar la cuenta cada vez que sea presionado.

Un pulsador conectado a `EINT2` permitirá modificar la velocidad de incremento del contador. En este sentido, cada vez que se presione este pulsador el contador pasará a incrementar su cuenta de cada `1` segundo a cada `1` milisegundo y viceversa.

Considerar que el microcontrolador se encuentra funcionando con un reloj (`cclk`) de `16 MHz`.

El código debe estar debidamente comentado y los cálculos realizados claramente expresados.

En la figura del ejercicio se muestra una tabla que codifica el display y el esquema de hardware sobre el que funcionará el programa.
