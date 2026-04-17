#include "LPC17xx.h"

uint8_t number; //uso 8 bits porque se que solo tengo que guardar 4 bits correspondientes a p2.0-p2.3 y esa es la menor cantidad de bits que puedo usar

void iniciarGPIO(void)
{
    LPC_PINCON -> PINSEL4 &= ~0xFF;
    LPC_GPIO2 -> FIODIR &= ~0xF;

    LPC_PINCON -> PINSEL4 &= ~(3 << 8);
    LPC_GPIO2 -> FIODIR &= ~(1 << 4);
    LPC_PINCON -> PINMODE4 &= ~(3 << 8); //PULLUP
}

void iniciarEINT3(void)
{
    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_SetPriority(EINT3_IRQn, 1);

    LPC_GPIOINT -> IO2IntEnF |= (1 << 4);
    LPC_GPIOINT -> IO2IntClr = (1 << 4);
}

void EINT3_IRQHandler(void)
{
    number = (LPC_GPIO2 -> FIOPIN & 0xF);

    number = ((number & 0x1) << 3) | ((number & 0x2) << 1) | ((number & 0x4) >> 1) | ((number & 0x8) >> 3);

    LPC_GPIOINT -> IO2IntClr = (1 << 4);
}

int main(void)
{
    iniciarGPIO();
    iniciarEINT3();

    while(1)
    {

    }
}