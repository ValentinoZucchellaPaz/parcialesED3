#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_timer.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_gpdma.h"

uint32_t muestras[100] = {0};
uint32_t promedio = 0;

void initADC(void)
{
    ADC_Init(200000);
    ADC_PinConfig(4); //p1.30 es del canal 4 del adc
    ADC_PowerUp();
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_EdgeStartConfig(ADC_START_ON_FALLING);
    ADC_ChannelEnable(4);
}

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
    mat01.matchValue = 50; //100us para 10khz, 50 porque adc toma el falling

    TIM_InitTimer(LPC_TIM0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) &(LPC_ADC -> ADDR4);
    lli.dstAddr = (uint32_t) &muestras;
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 100 | (2 << 18) | (2 << 21) | (1 << 27) | (1 << 31); //el burst del adc podria ser 16?

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 100;
    dma.type = GPDMA_P2M;
    dma.scrMemAddr = 0;
    dma.dstMemAddr = (uint32_t) &muestras;
    dma.scrConn = GPDMA_ADC;
    dma.dstConn = 0;
    dma.scr.width = GPDMA_WORD; //revisar
    dma.scr.burst = GPDMA_BSIZE_1;
    dma.scr.increment = DISABLE;
    dma.dst.width = GPDMA_WORD; 
    dma.dst.burst = GPDMA_BSIZE_1;
    dma.dst.increment = ENABLE;
    dma.intTC = ENABLE;
    dma.intErr = DISABLE;
    dma.linkedList = (uint32_t) &lli;

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPMDA_ChannelStart(0);
    NVIC_EnableIRQ(DMA_IRQn);
}

void DMA_IRQHandler(void)
{
    uint32_t sumatoria = 0;

    GPDMA_ChannelGracefulStop(0);
    for(int i = 0; i < 100; i++)
    {
        sumatoria += (muestras[i] >> 4)&0xFFF;
    }
    GPDMA_ChannelStart(0);

    promedio = ((sumatoria/100) >> 2) << 6; // De 12 bits a 10 bits, adaptado para dac
    DAC_UpdateValue(promedio);
    GPDMA_ClearIntPending(GPDMA_CLR_INTTC, 0);
}

void initDAC(void)
{
    PINSEL_CFG_T pinDac;
    pinDac.port = 0;
    pinDac.pin = 26;
    pinDac.func = PINSEL_FUNC_10;
    pinDac.mode = PINSEL_TRISTATE;
    pinDac.openDrain = DISABLE;

    PINSEL_ConfigPin(&pinDac);
    DAC_Init();
}

int main(void)
{
    initTimer0();
    initADC();
    initGPDMA();
    initDAC();

    while(1)
    {

    }
}
