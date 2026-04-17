#include "LPC17xx.h"

void iniciarTIM0(void)
{
    LPC_SC -> PCONP |= (1 << 1); //LE DAMOS ENERGIA AL TIMER
    
    LPC_SC -> PCLKSEL0 &= ~(3 << 2); //PCLK/4 = 25MHZ
    LPC_TIM0 -> PR = 0; //SIN PREESCALER

    LPC_TIM0 -> MR0 = 25000000; //1SEG
    LPC_TIM0 -> MCR = (1 << 0) | (1 << 1); //INTERRUPCION + RESET AL HACER MATCH

    NVIC_EnableIRQ(TIMER0_IRQn);
    LPC_TIM0 -> TCR = (1 << 1); //RESET TIMER
    LPC_TIM0 -> TCR = (1 << 0); //ENABLE TIMER
}

void TIMER0_IRQHandler(void)
{
    if(LPC_TIM0 -> IR & (1 << 0)) //REVISAMOS SI LA INT ES DEL MR0
    {
        LPC_GPIO0 -> FIOPIN ^= (1 << 22);
        LPC_TIM0 -> IR = (1 << 0); //LIMPIAMOS FLAG QUE USAMOS (POR ESO VA DENTRO DEL IF)
    }
}

void iniciarGPIO(void)
{
    LPC_PINCON -> PINSEL1 &= ~(3 << 12);
    LPC_GPIO0 -> FIODIR |= (1 << 22);
    LPC_GPIO0 -> FIOCLR = (1 << 22);
}

int main(void)
{
    iniciarGPIO();
    iniciarTIM0();

    while(1)
    {

    }
}