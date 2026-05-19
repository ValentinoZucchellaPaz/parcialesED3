#include "LPC17xx.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_pinsel.h"

uint32_t* src = (uint32_t*) 0x10004000;

uint32_t DAC_Max = *src & 0xFF;
uint32_t valMax_t = (*src >> 8) 0xFF;
uint32_t subida_t = (*src >> 16) 0xFF;
uint32_t bajada_t = (*src >> 24) 0xFF;

void demora(uint32_t x)
{
    for(int i = 0; i < x * 100 ; i++)
    {

    }
}

void initPin(void)
{
    PINSEL_CFG_T pin;
    pin.port = 0;
    pin.pin = 26;
    pin.func = PINSEL_FUNC_10;
    pin.mode = PINSEL_TRISTATE;
    pin.openDrain = DISABLE;
    PINSEL_ConfigPin(&pin);
}

int main(void)
{
    DAC_Init();

    while(1)
    {
        for(uint32_t i = 0 ; i < DAC_Max ; i++)
        {
            DAC_UpdateValue(i);
            demora(subida_t);
        }

        demora(valMax_t);

        for(uint32_t i = DAC_Max; i > 0; i--)
        {
            DAC_UpdateValue(i);
            demora(bajada_t);
        }
    }
}