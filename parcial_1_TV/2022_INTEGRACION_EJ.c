/*
===============================================================================
 Alumno		: Marclé Emiliano
 Carrera	: Ing. Electronica
 Descripcion : Algoritmo de antirrebote de un pulsador: Escribir un programa en C que ante la interrupción por
 flanco de subida del pin P0.1, configurado como entrada digital con pull-down interno, se incremente un contador de un dígito,
 se deshabilite esta interrupción y se permita la interrupción por systick cada 20 milisegundos.
 En cada interrupción del systick se testeará una vez el pin P0.1.
 Solo para el caso de haber testeado 3 estados altos seguidos se sacará por los pines del puerto P2.0 al P2.7
 el equivalente en ascii del valor del contador, se desactivará las interrupción por systick y se habilitará nuevamente la interrupción por P0.1.
 Por especificación de diseño se pide que los pines del puerto 2 que no sean utilizados deben estar enmascarados por hardware.
 Considerar que el CPU se encuentra funcionando con el oscilador interno RC (4Mhz).
===============================================================================
*/

#include "LPC17xx.h"

volatile uint8_t intSys = 0;
volatile uint8_t intEINT = 1;
volatile uint8_t contador = 0;
uint8_t cantAltos = 0;

void iniciarGPIO(void)
{
    //P0.01 COMO ENTRADA E EINT3
    LPC_PINCON -> PINSEL0 &= ~(3 << 2);
    LPC_GPIO0 -> FIODIR &= ~(1 << 1);
    LPC_PINCON -> PINMODE &= ~(3 << 2);
    LPC_PINCON -> PINMODE |= (2 << 2); //pull down interno

    //P2.00 a P2.07 COMO SALIDAS
    LPC_PINCON -> PINSEL4 &= ~0xFFFF;
    LPC_GPIO2 -> FIODIR |= 0xFF;
    //me piden enmascaracion por hardware asi que no debo hacer nada de codigo
}

void iniciarSysTick(void)
{
    SysTick -> LOAD = ((4000000/1000 * 20) - 1); //20ms
    SysTick -> VAL = 0;
    SysTick -> CTRL |= 7;
    SysTick -> CTRL &= ~(1 << 1); //deshabilito interrupcion
    intSys = 0;
}

void iniciarEINT3(void)
{
    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_SetPriority(EINT3_IRQn, 1);

    LPC_GPIOINT -> IO0IntEnR |= (1 << 1);
    LPC_GPIOINT -> IO0IntClr = (1 << 1);
}

void EINT3_IRQHandler(void)
{
    contador++;

    if(contador >= 10)
    {
        contador = 0;
    }

    NVIC_DisableIRQ(EINT3_IRQn);
    intEINT = 0;

    SysTick -> CTRL |= (1 << 1);
    intSys = 1;

    LPC_GPIOINT -> IO0IntClr = (1 << 1);
}

void SysTick_Handler(void)
{
    if(LPC_GPIO0 -> FIOPIN & (1 << 1))
    {
        cantAltos++;
    }
    else
    {
        cantAltos = 0;
    }

    if(cantAltos == 3)
    {
        cantAltos = 0;

        LPC_GPIO2 -> FIOCLR = 0xFF;
        LPC_GPIO2 -> FIOSET = contador + 48; //valor ascii

        SysTick -> CTRL &= ~(1 << 1);
        intSys = 0;

        NVIC_EnableIRQ(EINT3_IRQn);
        intEINT = 1;
    }
}

int main(void)
{
    iniciarGPIO();
    iniciarSysTick();
    iniciarEINT3();

    while(1)
    {

    }
}
