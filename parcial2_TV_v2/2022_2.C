#include "LPC17xx.h"
#include "LCP17xx_timer.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_pinsel.h"

volatile uint8_t subiendo = 1;
uint32_t DACValue = 0;


void initPin(void)
{
    PINSEL_CFG_T dacPin;
    dacPin.port = 0;
    dacPin.pin = 26;
    dacPin.func = PINSEL_FUNC_10;
    dacPin.mode = PINSEL_TRISTATE;
    dacPin.openDrain = DISABLE;
    PINSEL_ConfigPin(&dacPin);
}

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
    mat00.matchValue = 1; //inhumanamente rapido, quizas hasta colapse el lpc, convendria mas usar 250 ticks para 10us en 25mhz

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat00);
    TIM_Enable(LPC_TIM0);
    NVIC_EnableIRQ(TIM0_IRQn);
}

void initDAC(void)
{
    DAC_Init();
}

void TIMER0_IRQHandler(void)
{
    if(TIM_GetIntStatus(LPC_TIM0, TIM_MR0_INT))
    {
        if(subiendo == 1)
        {
            DAC_UpdateValue(DACValue);
            DACValue++;

            if(DACValue >= 1023)
            {
                DACValue = 1022;
                subiendo = 0;
            }
        }
        else if(subiendo == 0)
        {
            DAC_UpdateValue(DACValue);
            DACValue--;

            if(DACValue == 0)
            {
                DACValue = 0;
                subiendo = 1;
            }
        }

        TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
    }
}

int main(void)
{
    initPin();
    initDAC();
    initTimer0();

    while(1)
    {

    }
}