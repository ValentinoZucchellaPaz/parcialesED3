/*
 * Generador de Secuencia Temporal con EINT2 y SysTick
 * 
 * Descripción:
 * - EINT2 (flanco descendente) inicia la secuencia en P2.4
 * - Secuencia: 40ms bajo, 4 pulsos de 10ms, 40ms bajo
 * - Nueva interrupción durante secuencia: aborta y pone P2.4 en alto
 * - Sin retardos por software, solo SysTick
 * - Frecuencia del sistema: 60 MHz
 */

#include "LPC17xx.h"

typedef enum
{
    idle,
    firstLow,
    pulseHigh,
    pulseLow,
    finalLow
} Estado;

volatile Estado estado = idle;
uint8_t pulsos = 0;
volatile uint32_t t = 0;
uint32_t t_ref = 0;

void iniciarGPIO(void)
{
    //P2.04 SALIDA
    LPC_PINCON -> PINSEL4 &= ~(3 << 8);
    LPC_GPIO2 -> FIODIR |= (1 << 4);
    LPC_GPIO2 -> FIOSET = (1 << 4); //empezamos en alto
}

void iniciarEINT2(void)
{
    LPC_PINCON -> PINSEL4 &= ~(3 << 24);
    LPC_PINCON -> PINSEL4 |= (1 << 24);

    NVIC_EnableIRQ(EINT2_IRQn);
    NVIC_SetPriority(EINT2_IRQn, 1);

    LPC_SC -> EXTMODE |= (1 << 2); //POR FLANCO
    LPC_SC -> EXTPOLAR &= ~(1 << 2); //FLANCO DESCENDENTE
    LPC_SC -> EXTINT = (1 << 2);
}

void iniciarSysTick(void)
{
    SysTick -> LOAD = (60000000/1000) - 1; //1ms
    SysTick -> VAL = 0;
    SysTick -> CTRL |= 7;
}

void SysTick_Handler(void)
{
    t++;
}

void EINT2_IRQHandler(void)
{
    if(estado == idle)
    {
        LPC_GPIO2 -> FIOCLR = (1 << 4);
        t_ref = t;
        estado = firstLow;
    }
    else
    {
        estado == idle;
    }

    LPC_SC -> EXTINT = (1 << 2);
}

int main(void)
{
    iniciarGPIO();
    iniciarSysTick();
    iniciarEINT2();

    while(1)
    {
        switch(estado)
        {
            case idle:
            LPC_GPIO2 -> FIOSET = (1 << 4); //si lo hiciesemos en el handler seria unos ms mas rapido, en la practica no pasa nada
            t = 0;
            t_ref = 0;
            pulsos = 0;
            break;

            case firstLow:
            if(t - t_ref >= 40)
            {
                LPC_GPIO2 -> FIOSET = (1 << 4);
                t_ref = t;
                estado = pulseHigh;
            }
            break;

            case pulseHigh:
            if(t - t_ref >= 10)
            {
                LPC_GPIO2 -> FIOCLR = (1 << 4);
                t_ref = t;
                pulsos++;

                if(pulsos >= 4)
                {
                    estado = finalLow;
                }
                else
                {
                    estado = pulseLow;
                }
            }
            break;

            case pulseLow:
            if(t - t_ref >= 10)
            {
                LPC_GPIO2 -> FIOSET = (1 << 4);
                t_ref = t;
                estado = pulseHigh;
            }
            break;

            case finalLow:
            if(t - t_ref >= 40)
            {
                LPC_GPIO2 -> FIOSET = (1 << 4);
                estado = idle;
            }
            break;
        }
    }
}