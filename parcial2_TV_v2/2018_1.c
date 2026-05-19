#include "LPC17xx.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_gpdma.h"

uint32_t* valoresDAC = 0x10004000;

uint32_t valor_maximo = *valoresDAC & (0xFF);
uint32_t tiempo_demora = (*valoresDAC >> 8) & (0xFF);
uint32_t tiempo_subida = (*valoresDAC >> 16) & (0xFF);
uint32_t tiempo_bajada = (*valoresDAC >> 24) & (0xFF); 

void demora(uint32_t t)
{
    for(int i = 0; i < t*100; i++)
    {
        
    }
}

void initPin(void)
{
    //P0.26 como DAC
    PINSEL_CFG_T dacPin;
    dacPin.port = 0;
    dacPin.pin = 26;
    dacPin.func = PINSEL_FUNC_10;
    dacPin.mode = PINSEL_TRISTATE;
    dacPin.openDrain = DISABLE;
    PINSEL_ConfigPin(&dacPin);
}

void initDAC(void)
{
    DAC_Init();
}

int main(void)
{
    initPin();
    initDAC();

    while(1)
    {
        for(uint32_t i = 0; i <= valor_maximo; i++)
        {
            DAC_UpdateValue(i);
            demora(tiempo_subida);
        }

        demora(tiempo_demora);

        for(uint32_t i = valor_maximo; i > 0; i--)
        {
            DAC_UpdateValue(i);
            demora(tiempo_bajada);
        }
    }
}