#include "LPC17xx.h"
#include "LPC17xx_timer.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_pinsel.h"

uint32_t dacValue = 0;
uint8_t subiendo = 1;

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_TICK;
    tim0.prescaleValue = 0;

    TIM_MATCHCFG_T mat00;
    mat00.channel = 0;
    mat00.intEn = ENABLE;
    mat00.stopEn = DISABLE;
    mat00.resetEn = ENABLE;
    mat00.extOpt = TIM_NOTHING;
    mat00.matchValue = 25;
    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat00);
    NVIC_EnableIRQ(TIMER0_IRQn);
    TIM_Enable(LPC_TIM0);
}

void initDAC(void)
{
    DAC_Init();

    PINSEL_CFG_T pin;
    pin.port = 0;
    pin.pin = 26;
    pin.func = PINSEL_FUNC_10;
    pin.mode = PINSEL_TRISTATE;
    pin.openDrain = DISABLE;
    PINSEL_ConfigPin(&pin);
}

void TIMER0_IRQHandler(void)
{
    if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT)){
    if(subiendo == 1)
    {
        DAC_UpdateValue(dacValue);
        dacValue++;
        if(dacValue >= 1023)
        {
            dacValue = 1022;
            subiendo = 0;
        }
    }
    else if(subiendo == 0)
    {
        DAC_UpdateValue(dacValue);
        dacValue--;
        if(dacValue == 0)
        {
            dacValue = 0;
            subiendo = 1;
        }
    }

    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT)
    }
}

int main(void)
{
    initTimer0();
    initDAC();

    while(1)
    {

    }
}

