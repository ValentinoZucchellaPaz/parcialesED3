#include "LPC17xx.h"

volatile uint8_t output = 0xA; // ejemplo

void iniciarSysTick(void)
{
    SysTick -> LOAD = ((80000000/100) - 1); //10ms
    SysTick -> VAL = 0;
    SysTick -> CTRL |= 7;

    NVIC_SetPriority(SysTick_IRQn, 0);
}

void iniciarEINT1(void)
{
    LPC_PINCON -> PINSEL4 &= ~(3 << 22);
    LPC_PINCON -> PINSEL4 |= (1 << 22); //P2.11 COMO EINT1
    LPC_PINCON -> PINMODE4 &= ~(1 << 22); //PULL UP

    LPC_SC -> EXTMODE |= (1 << 1);
    LPC_SC -> EXTPOLAR &= ~(1 << 1); //POR BAJADA DE FLANCO
    LPC_SC -> EXTINT = (1 << 1);

    NVIC_EnableIRQ(EINT1_IRQn);
    NVIC_SetPriority(EINT1_IRQn, 1);
}

void iniciarGPIO(void)
{
    LPC_PINCON -> PINSEL0 &= ~0xFF;
    LPC_GPIO0 -> FIODIR |= 0xF;
}

void EINT1_IRQHandler(void)
{
    SysTick -> VAL = 0;

    LPC_GPIO0 -> FIOCLR = 0xF;
    LPC_GPIO0 -> FIOSET = output & 0xF;

    LPC_SC -> EXTINT = (1 << 1);
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