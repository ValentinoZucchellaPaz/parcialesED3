#include "LPC17xx.h"
#include "LPC17xx_timer.h"
#include "LPC17xx_adc.h"

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.presacaleOpt = TIM_US;
    tim0.prescaleValue = 1000;

    TIM_MATCHCFG_T mat01;
    mat01.channel = 1;
    mat01.intEn = DISABLE;
    mat01.stopEn = DISABLE;
    mat01.resetEn = ENABLE;
    mat01.extOpt = TIM_TOGGLE;
    mat01.matchValue = 5; //10ms para 100hz de frec. muestreo, 5ms pq solo tomo el falling edge

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initADC(void)
{
    ADC_Init(200000);
    ADC_PinConfig(0);
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_ChannelEnable(0);
    ADC_EdgeStartConfig(ADC_START_ON_FALLING);
    ADC_IntEnable(0);
    NVIC_EnableIRQ(ADC_IRQn);
    ADC_PowerUp();
}


uint32_t muestras_diez[20] = {0};
uint8_t numMuestras = 0;
uint8_t indice = 0;

numMuestras ++;

if(numMuestras >= N)
{
    numMuestras = 0;
    muestras_diez[indice] = filtSignal;
    indice++;
    if(indice >= 20) {indice = 0;}
}