/*
 * Parpadeo de LED con período proporcional a señal de entrada
 * 
 * Descripción:
 * - Mide el período de una señal cuadrada en EINT1 (P2.11)
 * - LED en P1.18 parpadea con período proporcional (100-1000ms)
 * - SysTick como base de tiempo para medición y control
 */

#include "LPC17xx.h"

volatile uint32_t t = 0;
uint32_t t_ref = 0;
uint32_t t_refLed = 0;
uint32_t periodo = 500;
volatile uint8_t midiendo = 0;

void iniciarGPIO(void)
{
    //P1.18 COMO SALIDA
    LPC_PINCON -> PINSEL3 &= ~(3 << 4);
    LPC_GPIO1 -> FIODIR |= (1 << 18);
    LPC_GPIO1 -> FIOCLR = (1 << 18);
}

void iniciarSysTick(void)
{
    SysTick -> LOAD = (SystemCoreClock/1000) - 1; //1ms
    SysTick -> VAL = 0;
    SysTick -> CTRL |= 7;
}

void iniciarEINT1(void)
{
    NVIC_EnableIRQ(EINT1_IRQn);
    NVIC_SetPriority(EINT1_IRQn, 1);

    LPC_PINCON -> PINSEL4 &= ~(3 << 22);
    LPC_PINCON -> PINSEL4 |= (1 << 22);
    LPC_PINCON -> PINMODE4 &= ~(3 << 22);
    LPC_PINCON -> PINMODE4 |= (2 << 22); //PULL DOWN
    
    LPC_SC -> EXTMODE |= (1 << 1);
    LPC_SC -> EXTPOLAR |= (1 << 1); //medimos tiempo entre flancos de subida del boton
    LPC_SC -> EXTINT = (1 << 1);
}

void SysTick_Handler(void)
{
    t++;
}

void EINT1_IRQHandler(void)
{
    if(midiendo == 0)
    {
        t_ref = t;
        midiendo = 1;
    }
    else
    {
        midiendo = 0;
        periodo = ((t - t_ref) / 2);

        if(periodo < 100)
        {
            periodo = 100;
        }

        if(periodo > 1000)
        {
            periodo = 1000;
        }
    }

    LPC_SC -> EXTINT = (1 << 1);
}

int main(void)
{
    iniciarGPIO();
    iniciarSysTick();
    iniciarEINT1();

    while(1)
    {
        if(t - t_refLed >= periodo)
        {
            LPC_GPIO1 -> FIOPIN ^= (1 << 18); //toggle prendido y apagado
            t_refLed = t;
        }
    }
}
