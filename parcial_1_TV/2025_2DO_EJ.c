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

 /*
 * Generador de Secuencia Temporal -  con Array
 * 
 * Descripción:
 * - Cada elemento del array representa 10ms
 * - Valor 0 = P2.4 en bajo, Valor 1 = P2.4 en alto
 * - EINT2 inicia la secuencia
 * - Nueva interrupción aborta y pone P2.4 en alto
 * - Frecuencia del sistema: 60 MHz
 */

#include "LPC17xx.h"

uint8_t sec[] = {0,0,0,0,1,0,1,0,1,0,1,0,0,0,0};
volatile uint32_t t = 0;
uint32_t t_ref = 0;
uint8_t index = 0;
volatile uint8_t secuenciaActiva = 0, abort = 0;

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
    if(secuenciaActiva == 0)
    {
        secuenciaActiva = 1;
        abort = 0;

        LPC_GPIO2 -> FIOCLR = (1 << 4);
        index = 1;
        t_ref = t;
    }
    else if (abort == 0)
    {
        abort = 1;
        secuenciaActiva = 0;
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
        if(secuenciaActiva)
        {
            if(t - t_ref >= 10)
            {
                if(sec[index] == 0)
                {
                    LPC_GPIO2 -> FIOCLR = (1 << 4);
                }
                else
                {
                    LPC_GPIO2 -> FIOSET = (1 << 4);
                }

                t_ref = t;
                index++;

                if(index >= 15)
                {
                    index = 0;
                    secuenciaActiva = 0;
                    LPC_GPIO2 -> FIOSET = (1 << 4);
                }
            }
        }
        else if(abort == 1)
        {
            LPC_GPIO2 -> FIOSET = (1 << 4);
            index = 0;
            abort = 0;
        }
    }
}
