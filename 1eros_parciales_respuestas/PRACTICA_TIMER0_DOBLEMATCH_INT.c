#include "LPC17xx.h"

void iniciarTIM0(void)
{
    NVIC_EnableIRQ(TIMER0_IRQn);
    
    LPC_SC -> PCONP |= (1 << 1); //LE DAMOS ENERGIA AL TIMER
    
    LPC_SC -> PCLKSEL0 &= ~(3 << 2); //PCLK/4 = 25MHZ
    LPC_TIM0 -> PR = 0; //SIN PREESCALER

    LPC_TIM0 -> MR0 = 25000000; //1SEG
    LPC_TIM0 -> MR1 = 50000000; //2SEG
    LPC_TIM0 -> MCR = (1 << 0) | (1 << 3) | (1 << 4); //INTERRUPCION 0 + INTERRUPCION 1 + RESET AL HACER MATCH 1

    LPC_TIM0 -> TCR = (1 << 1); //RESET TIMER
    LPC_TIM0 -> TCR = (1 << 0); //ENABLE TIMER
}

void TIMER0_IRQHandler(void)
{
    if(LPC_TIM0 -> IR & (1 << 0)) //REVISAMOS SI LA INT ES DEL MR0
    {
        LPC_GPIO0 -> FIOSET = (1 << 22);
        LPC_TIM0 -> IR = (1 << 0); //LIMPIAMOS FLAG QUE USAMOS (POR ESO VA DENTRO DEL IF)
    }
    
    if(LPC_TIM0 -> IR & (1 << 1)) //REVISAMOS SI LA INT ES DEL MR1
    {
        LPC_GPIO0 -> FIOCLR = (1 << 22);
        LPC_TIM0 -> IR = (1 << 1); //LIMPIAMOS FLAG QUE USAMOS (POR ESO VA DENTRO DEL IF)
    }

    //no usamos else if porque si llego al mr1 entonces tmb llego al mr0, por eso revisamos bits individuales y no conjuntos
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