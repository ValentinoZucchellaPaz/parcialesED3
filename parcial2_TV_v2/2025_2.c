#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_timer.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio"

volatile uint32_t lecturas[4] = {0};
volatile uint8_t indice = 0;
uint32_t sumatoria = 0;
uint32_t promedio = 0;

void initADC(void)
{
    ADC_Init(200000);
    ADC_PinConfig(0);
    ADC_PowerUp();
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_ChannelEnable(0);
}

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 1000; //1ms

    TIM_MATCHCFG_T mat01;
    mat01.channel = 1;
    mat01.intEn = DISABLE;
    mat01.stopEn = DISABLE;
    mat01.resetEn = ENABLE;
    mat01.extOpt = TIM_TOGGLE;
    mat01.matchValue = 30000; //30seg

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initGPDMA(void)
{
    static GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) &(LPC_ADC -> ADDR0);
    lli.dstAddr = (uint32_t) &(lecturas);
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 4 | (2 << 18) | (2 << 21) | (1 << 27);

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 4;
    dma.type = GPDMA_P2M;
    dma.scrMemAddr = 0;
    dma.dstMemAddr = (uint32_t) &(lecturas);
    dma.scrConn = GPDMA_ADC;
    dma.dstConn = 0;
    dma.src.width = GPDMA_WORD;
    dma.src.burst = GPDMA_BSIZE_1;
    dma.src.increment = DISABLE;
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
    tim1.prescaleValue = 10000; //10ms

    TIM_MATCHCFG_T mat10;
    mat10.channel = 0;
    mat10.intEn = ENABLE;
    mat10.stopEn = DISABLE;
    mat10.resetEn = ENABLE;
    mat10.extOpt = TIM_NOTHING;
    mat10.matchValue = 12100; //2min + 1seg por las dudas

    TIM_InitTimer(LPC_TIM1, &tim1);
    TIM_ConfigMatch(LPC_TIM1, &mat10);
    TIM_Enable(LPC_TIM1);
    NVIC_EnableIRQ(TIMER1_IRQn);
}

void TIMER1_IRQHandler(void)
{
    if(TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT))
    {
        sumatoria = 0;

        for(int i = 0; i < 4; i++)
        {
            sumatoria = sumatoria + (lecturas[i] >> 4)&0xFFF;
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
    tim2.prescaleValue = 1000; //1ms

    TIM_MATCHCFG_T mat20;
    mat20.channel = 0;
    mat20.intEn = ENABLE;
    mat20.stopEn = DISABLE;
    mat20.resetEn = ENABLE;
    mat20.extOpt = TIM_NOTHING;
    mat20.matchValue = 100; //100ms

    TIM_InitTimer(LPC_TIM2, &tim2);
    TIM_ConfigMatch(LPC_TIM2, &mat20);
    //no hacemos el enable, eso depende del main
}

//toggle pin

int main(void)
{
    initADC();
    initTimer0();
    initGPDMA();
    initTimer1();
    initPin();

    while(1)
    {
        if(promedio > 0 && promedio < 1241) //1V siendo que 3,3V=4095
        {
            TIM_Disable(LPC_TIM2);
            GPIO_ClearPins(0, (1 << 22));
        }
        else if(promedio >= 1241 && promedio <= 2482)
        {
            TIM_Enable(LPC_TIM2);
        }
        else if (promedio > 2482)
        {
            TIM_Disable(LPC_TIM2);
            GPIO_SetPinState(0, 22, SET);
        }
    }
}