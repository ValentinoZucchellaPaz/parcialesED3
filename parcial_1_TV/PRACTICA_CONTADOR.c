#include "LPC17xx.h"

void iniciarGPIO(void)
{
    LPC_PINCON -> PINSEL1 &= ~(3 << 12);
    LPC_GPIO0 -> FIODIR |= (1 << 22);
    LPC_GPIO0 -> FIOCLR = (1 << 22);
}

void iniciarContador(void)
{
    LPC_SC -> PCONP |= (1 << 1);
    LPC_TIM0 -> PR = 0;

    LPC_TIM0 -> CTCR = 1; //CONTAR FLANCOS ALTOS

    LPC_PINCON -> PINSEL3 &= ~(3 << 20);
    LPC_PINCON -> PINSEL3 |= (3 << 20); //CAP0.0

    LPC_TIM0 -> MR0 = 100;
    LPC_TIM0 -> MCR = (1 << 0) | (1 << 1);

    LPC_TIM0 -> TCR = (1 << 1);
    LPC_TIM0 -> TCR = (1 << 0);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

void TIMER0_IRQHandler(void)
{
    if(LPC_TIM0 -> IR & (1 << 0))
    {
        LPC_GPIO0 -> FIOPIN ^= (1 << 22);
        LPC_TIM0 -> IR = (1 << 0);
    }
}

int main(void)
{
    iniciarGPIO();
    iniciarContador();

    while(1)
    {

    }
}