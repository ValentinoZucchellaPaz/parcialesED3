#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_timer.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"

volatile uint32_t datos[4] = {0};
uint32_t promedio = 0;

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
    mat01.matchValue = 15000; //uso 15seg porque en el adc solo tomo el edge_falling, de esta forma hago 30seg

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initADC(void)
{
    ADC_Init();
    ADC_PinConfig(0);
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_EdgeStartConfig(ADC_START_ON_FALLING);
    ADC_ChannelEnable(0);
    ADC_PowerUp();
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli;
    lli.srcAddr =  (uint32_t) &(LPC_ADC -> ADDR0);
    lli.dstAddr = (uint32_t) &datos;
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 4 | (2 << 18) | (2 << 21) | (2 << 27);

    GPDMA_Channel_CFG_T dma = {
        .channelNum = 0,
        .transferSize = 4,
        .type = GPDMA_P2M,
        .srcMemAddr = 0,
        .dstMemAddr = (uint32_t) &datos,
        .scrConn = GPDMA_ADC,
        .dstConn = 0,
        .src = {GPDMA_WORD, GPDMA_BSIZE_1, DISABLE},
        .dst = {GPDMA_WORD, GPDMA_BSIZE_1, ENABLE},
        .intTC = DISABLE,
        .intErr = DISABLE,
        .linkedList = (uint32_t) &lli
    };

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
    mat10.matchValue = 12100; //2min + 1seg para asegurar la conversion correcta

    TIM_InitTimer(LPC_TIM1, &tim1);
    TIM_ConfigMatch(LPC_TIM1, &mat10);
    NVIC_EnableIRQ(TIMER1_IRQn)
    TIM_Enable(LPC_TIM1);
}

void TIMER1_IRQHandler(void)
{
    if(TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT))
    {
    uint32_t sumatoria = 0;

    for(uint32_t i = 0; i < 4; i++)
    {
        sumatoria += (datos[i] >> 4)&0xFFF;
    }

    promedio = sumatoria/4;

    TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    }
}

void initPin(void)
{
    PINSEL_CFG_T pin;
    pin.port = 0;
    pin.pin = 22;
    pin.func = PINSEL_FUNC_00;
    pin.mode = PINSEL_TRISTATE;
    pin.openDrain = DISABLE;
    PINSEL_ConfigPin(&pin);
    GPIO_SetDir(0, (1 << 22), GPIO_OUTPUT);
}

void initTimer2(void)
{
    TIM_TIMERCFG_T tim2;
    tim2.prescaleOpt = TIM_US;
    tim2.prescaleValue = 1000;

    TIM_MATCHCFG_T mat20;
    mat20.channel = 0;
    mat20.intEn = ENABLE;
    mat20.stopEn = DISABLE;
    mat20.resetEn = ENABLE;
    mat20.extOpt = TIM_NOTHING;
    mat20.matchValue = 100;

    TIM_InitTimer(LPC_TIM2, &tim2);
    TIM_ConfigMatch(LPC_TIM2, &mat20);
    NVIC_EnableIRQ(TIMER2_IRQn);
}

void TIMER2_IRQn(void)
{
    if(TIM_GetIntStatus(LPC_TIM2, TIM_MR0_INT))
    {
        if(GPIO_ReadValue(0) & (1 << 22))
        {
            GPIO_SetPinState(0, 22, CLEAR);
        }
        else
        {
            GPIO_SetPinState(0, 22, SET);
        }

        TIM_ClearIntPending(LPC_TIM2, TIM_MR0_INT);
    }
}

int main(void)
{
    initTimer0();
    initADC();
    initGPDMA();
    initTimer1();
    initPin();
    initTimer2();

    while(1)
    {
        if(promedio > 0 && promedio < 1241)
        {
            TIM_Disable(LPC_TIM2);
            GPIO_SetPinState(0, 22, CLEAR);
        }
        else if(promedio >= 1241 && promedio <= 2482)
        {
            TIM_Enable(LPC_TIM2);
        }
        else if(promedio > 2482)
        {
            TIM_Disable(LPC_TIM2);
            GPIO_SetPinState(0, 22, SET);
        }
    }
}