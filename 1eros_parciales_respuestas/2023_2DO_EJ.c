#include "LPC17xx.h"

uint8_t memoria[10];
uint8_t lectura = 0;
uint8_t leerFlag = 0;
uint8_t prioridad = 0;
uint8_t cantNums = 0;

void iniciarGPIO(void)
{
    LPC_PINCON -> PINSEL0 &= ~0xFF; //P0.00 A P0.03
    LPC_GPIO0 -> FIODIR &= ~0xF;
    LPC_PINCON->PINMODE0 &= ~0xFF;
    LPC_PINCON->PINMODE0 |= 0xAA; //sin resistencias internas (2)
}

void iniciarEINT3(void)
{
    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_SetPriority(EINT3_IRQn, prioridad);

    LPC_GPIOINT -> IO0IntEnF |= 0xF;
    LPC_GPIOINT -> IO0IntClr = 0xF;
}

void EINT3_IRQHandler(void)
{
    lectura = (~LPC_GPIO0 -> FIOPIN) & 0xF; //invertimos pq hacemos pullup
    leerFlag = 1;
    LPC_GPIOINT -> IO0IntClr = 0xF;
}

int main(void)
{
    iniciarGPIO();
    iniciarEINT3();

    while(1)
    {
        if(leerFlag)
        {
            if(memoria[0] != lectura) //sugerencia gpt, evita duplidos, no se rompe la consigna si no esta
            {
                for(int i = 9; i > 0; i--)
                {
                    memoria[i] = memoria[i - 1];
                }

                memoria[0] = lectura;
                leerFlag = 0;

                cantNums++;
            }

            if(cantNums >= 200)
            {
                prioridad++;
                cantNums = 0;

                if(prioridad >= 31)
                {
                    NVIC_DisableIRQ(EINT3_IRQn);
                }
                else
                {
                    NVIC_SetPriority(EINT3_IRQn, prioridad);
                }
            }
        }
    }
}