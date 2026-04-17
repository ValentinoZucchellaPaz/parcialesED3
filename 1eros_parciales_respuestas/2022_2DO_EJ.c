/*
===============================================================================
 Name        : Ejercicio2_P1_2022.c
 Author      : $Marclé, Emiliano
 Version     : 1.0
 Description : Utilizando interrupciones por SysTick y por eventos externos EINT,
 realizar un programa que permita habilitar y deshabilitar el temporizador por flanco ascendente en el pin P2.11
 El temporizador debe desbordar cada 10 ms utilizando un clock de cclk=62MHz.
 Por cada interrupción del SysTick, se debe mostrar por el puerto P0 el promedio de los valores guardados en uint8_t values[8].
 Detallar cálculos para obtener el valor de RELOAD y asegurar que la interrupción por SysTick sea de mayor prioridad que EINT.

===============================================================================
*/

#include "LPC17xx.h"

uint8_t values [8] = {2,5,4,1,6,8,4,5}; //valores inventados por mi
uint16_t sumatoria = 0; //lo hacemos de 16 para evitar overflow
uint8_t promedio = 0;
uint8_t intSys = 0;

void iniciarGPIO(void)
{
    //P0.00 - P0.07 COMO SALIDAS
    LPC_PINCON -> PINSEL0 &= ~0xFFFF;
    LPC_GPIO0 -> FIODIR |= 0xFF;
}

void iniciarEINT1(void)
{
    NVIC_EnableIRQ(EINT1_IRQn);
    NVIC_SetPriority(EINT1_IRQn, 1);

    LPC_PINCON -> PINSEL4 &= ~(3 << 22);
    LPC_PINCON -> PINSEL4 |= (1 << 22); //P2.11 COMO EINT1

    LPC_PINCON -> PINMODE4 &= ~(3 << 22);
    LPC_PINCON -> PINMODE4 |= (2 << 22); //PULL DOWN

    LPC_SC -> EXTMODE |= (1 << 1); //FLANCO
    LPC_SC -> EXTPOLAR |= (1 << 1); //ASCENDENTE
    LPC_SC -> EXTINT = (1 << 1);
}

void iniciarSysTick(void)
{
    SysTick -> LOAD = ((62000000/100) - 1); //10ms
    SysTick -> VAL = 0;
    SysTick -> CTRL |= 7;
    SysTick -> CTRL &= ~(1 << 0); //deshabilitamos temporizador
    intSys = 0;

    NVIC_SetPriority(SysTick_IRQn, 0); // mayor prioridad
}

void EINT1_IRQHandler(void)
{
    if(intSys)
    {
        SysTick -> CTRL &= ~(1 << 0);
        intSys = 0;
    }
    else
    {
        SysTick -> CTRL |= (1 << 0);
        intSys = 1;
    }

    LPC_SC -> EXTINT = (1 << 1);
}

void SysTick_Handler(void)
{
    sumatoria = 0; //reiniciamos cada interrupcion de systick
    
    for(int i = 0; i < sizeof(values)/sizeof(values[0]); i++)
    {
        sumatoria = sumatoria + values[i];
    }

    promedio = sumatoria/(sizeof(values)/sizeof(values[0])); //se reinicia gracias al =

    LPC_GPIO0 -> FIOCLR = 0xFF;
    LPC_GPIO0 -> FIOSET = promedio;
}

int main(void)
{
    iniciarGPIO();
    iniciarSysTick();
    iniciarEINT1();

    while(1)
    {

    }
}