#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_gpdma"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"
#include "LPC17xx_timer.h"

volatile uint32_t muestras[4] = {0};
uint32_t promedio = 0;

void initADC(void)
{
    ADC_Init(200000);
    ADC_PinConfig(0);
    ADC_PowerUp();
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_ChannelEnable(0);
    ADC_EdgeStartConfig(ADC_START_ON_RISING);
}

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 1000;

    TIM_MATCHCFG_T mat01;
    mat01.channel = 1;
    mat01.intEn = DISABLE;
    mat01.stopEn = DISABLE;
    mat01.resetEn = ENABLE;
    mat01.extOpt = TIM_TOGGLE;
    mat01.matchValue = 30000;

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) &(LPC_ADC -> ADDR0);
    lli.dstAddr = (uint32_t) &muestras;
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 4 | (2 << 18) | (2 << 21) | (1 << 27);

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 4;
    dma.type = GPDMA_P2M;
    dma.scrMemAddr = 0;
    dma.dstMemAddr = (uint32_t) &muestras;
    dma.scrConn = GPDMA_ADC;
    dma.dstConn = 0;
    dma.src.width = GPDMA_WORD;
    dma.scr.burst = GPDMA_BSIZE_1;
    dma.scr.increment = DISABLE;
    dma.dst.width = GPDMA_WORD;
    dma.dst.burst = GPDMA_BSIZE_1;
    dma.dst.increment = ENABLE;
    dma.intTC = DISABLE;
    dma.intErr = DISABLE;
    dma.linkedList = (uint32_t) &lli;

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(0);
}

void initTimer1(void)
{
    TIM_TIMERCFG_T tim1;
    tim1.prescaleOpt = TIM_US;
    tim1.prescaleValue = 10000;

    TIM_MATCHCFG_T mat10;
    mat10.channel = 0;
    mat10.intEn = ENABLE;
    mat10.stopEn = DISABLE;
    mat10.resetEn = ENABLE;
    mat10.extOpt = TIM_NOTHING;
    mat10.matchValue = 12100; //2 min y 1 seg

    TIM_InitTimer(LPC_TIM1, &tim1);
    TIM_ConfigMatch(LPC_TIM1, &mat10);
    TIM_Enable(LPC_TIM1);
}

void TIMER1_IRQHandler(void)
{
    uint32_t sumatoria = 0;

    for(int i = 0; i < 4; i++)
    {
        sumatoria += (muestras[i] >> 4) & 0xFFF;
    }

    promedio = sumatoria/4

    TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
}

void initPin(void)
{
    PINSEL_CFG_T pin;
    pin.port = 2;
    pin.pin = 15;
    pin.func = PINSEL_FUNC_00;
    pin.mode = PINSEL_TRISTATE;
    pin.openDrain = DISABLE;
    PINSEL_ConfigPin(&pin);
    GPIO_SetDir(2, (1 << 15), GPIO_OUTPUT);
}

void initTimer2(uint32_t duty)
{
    TIM_TIMERCFG_T tim2;
    tim2.prescaleOpt = TIM_US;
    tim2.prescaleValue = 0;

    TIM_MATCHCFG_T mat20;
    mat20.channel = 0;
    mat20.intEn = ENABLE;
    mat20.stopEn = DISABLE;
    mat20.resetEn = ENABLE;
    mat20.extOp = TIM_NOTHING;
    mat20.matchValue = 50; //periodo de 20khz

    TIM_MATCHCFG_T mat21;
    mat21.channel = 1;
    mat21.intEn = ENABLE;
    mat21.stopEn = DISABLE;
    mat21.resetEn = DISABLE;
    mat21.extOpt = TIM_NOTHING;
    mat21.matchValue = duty;

    TIM_InitTimer(LPC_TIM2, &tim2);
    TIM_ConfigMatch(LPC_TIM2, &mat20);
    TIM_ConfigMatch(LPC_TIM2, &mat21);
}

void TIMER2_IRQHandler(void)
{
    if(TIM_GetIntStatus(LPC_TIM2, TIM_MR0_INT))
    {
        GPIO_SetPinState(2, 15, SET);
        TIM_ClearIntPending(LPC_TIM2, TIM_MR0_INT);
    }
    else if(TIM_GetIntStatus(LPC_TIM2, TIM_MR1_INT))
    {
        GPIO_SetPinState(2, 15, CLEAR);
        TIM_ClearIntPending(LPC_TIM2, TIM_MR1_INT);
    }
}

int main(void)
{
    initADC();
    initTimer0();
    initGPDMA();
    initTimer1();
    initTimer2();

    while(1)
    {
        if(promedio >= 0 && promedio < 1241)
        {
            TIM_Disable(LPC_TIM2);
            GPIO_SetPinState(2, 15, CLEAR);
        }
        else if(promedio >= 1241 && promedio <= 2482)
        {
            uint32_t duty = promedio * 45/1241;
            TIM_UpdateMatchValue(LPC_TIM2, 1, duty);
            TIM_Enable(LPC_TIM2);
        }
        else if(promedio > 2482)
        {
            TIM_Disable(LPC_TIM2);
            GPIO_SetPinState(2, 15, SET);
        }
    }
}