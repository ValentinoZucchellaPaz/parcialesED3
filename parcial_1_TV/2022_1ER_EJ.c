/*
===============================================================================
 Name        : Ejercicio1_P1_2022.c
 Author      : $Marclé, Emiliano
 Version     : 1.0
 Description : Realizar un programa que guarde mediante interrupción el dato presente
 en los pines de entrada P2.0 al P2.7 cada vez que se cambie su valor.
 Cada dato nuevo debe guardarse de forma consecutiva en una región de memoria que permita
 tener disponible siempre de los últimos 16 datos guardados.
===============================================================================
*/

#include "LPC17xx.h"

uint8_t memoria[16] = {0}; //evitamos datos basura
uint8_t index = 0;

void iniciarGPIO(void)
{
    //P2.00 - P2-07 COMO ENTRADAS E EINT
    LPC_PINCON -> PINSEL4 &= ~0xFFFF;
    LPC_GPIO2 -> FIODIR &= ~0xFF;
    LPC_PINCON -> PINMODE4 &= ~0xFFFF;
}

void iniciarEINT3(void)
{
    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_SetPriority(EINT3_IRQn, 1);

    LPC_GPIOINT -> IO2IntEnR |= 0xFF;
    LPC_GPIOINT -> IO2IntEnF |= 0xFF; //necesito detectar todos los cambios

    LPC_GPIOINT -> IO2IntClr = 0xFF;
}

void EINT3_IRQHandler(void)
{
    memoria[index] = (LPC_GPIO2 -> FIOPIN & 0xFF);
    index++;

    if(index >= 16)
    {
        index = 0;
    }

    LPC_GPIOINT -> IO2IntClr = 0xFF;
}

int main(void)
{
    iniciarGPIO();
    iniciarEINT3();

    while(1)
    {

    }
}
