#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_timer.h"

uint32_t adcCh2[20] = {0};
uint32_t adcCh4[20] = {0};
uint8_t i2 = 0;
uint8_t i4 = 0;

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 0;

    TIM_MATCHCFG_T mat01;
    mat01.channel = 1;
    mat01.intEn = DISABLE;
    mat01.stopEn = DISABLE;
    mat01.resetEn = ENABLE;
    mat01.extOpt = TIM_TOGGLE;
    mat01.matchValue = 12;

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initADC(void)
{
    ADC_Init();
    ADC_PinConfig(2);
    ADC_PinConfig(4);
    ADC_StartCmd(ADC_START_ON_MAT01)
    ADC_ChannelEnable(2);
    ADC_ChannelEnable(4);
    ADC_EdgeStartConfig(ADC_START_ON_FALLING);
    ADC_IntEnable(ADC_INT_CH2);
    ADC_IntEnable(ADC_INT_CH4);
    ADC_PowerUp();
    NVIC_Enable(ADC_IRQn);
}

void ADC_IRQHandler(void)
{
    if(ADC_ChannelGetStatus(2, ADC_DATA_DONE))
    {
        uint32_t adcData2 = ADC_ChannelGetData(2);

        adcCh2[i2] = adcData2;
        i2++;
        if(i2 >= 20) {i2=0;}
    }
    else if(ADC_ChannelGetStatus(4, ADC_DATA_DONE))
    {
        uint32_t adcData4 = ADC_ChannelGetData(4);

        adcCh4[i4] = adcData4;
        i4++;
        if(i4 >= 20) {i4=0;}
    }
}

int main(void)
{
    initTimer0();
    initADC();

    while(1)
    {

    }
}

