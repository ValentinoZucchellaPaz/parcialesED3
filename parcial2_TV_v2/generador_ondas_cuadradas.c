#include "LPC17xx.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"
#include "LPC17xx_gpdma"
#include "LPC17xx_timer.h"

uint32_t* wave = 0x2007C000;

void initWave(void)
{
    for(int i = 0; i < 16; i++) {
        wave[i] = (uint32_t)i; 
    }
}


void initPines(void)
{
    PINSEL_CFG_T pines;
    pines.port = 0;
    pines.func = PINSE_FUNC_00;
    pines.mode = PINSEL_TRISTATE;
    pines.openDrain = DISABLE;
    PINSEL_ConfigMultiplePins(&pines, 15);
    GPIO_SetDir(0, 15, GPIO_OUTPUT);
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) wave;
    lli.dstAddr = (uint32_t) &(LPC_GPIO0 -> FIOPIN);
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 16 | (2 << 18) | (2 << 21) | (1 << 26);

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 16;
    dma.type = GPDMA_M2P;
    dma.scrMemAddr = (uint32_t) wave;
    dma.dstMemAddr = 0;
    dma.scrConn = 0;
    dma.dstConn = GPDMA_MAT0_0;
    dma.scr.width = GPDMA_WORD;
    dma.scr.burst = GPDMA_BSIZE_1;
    dma.scr.increment = ENABLE;
    dma.dst.width = GPDMA_WORD;
    dma.dst.burst = GPDMA_BSIZE_1;
    dma.dst.increment = DISABLE;
    dma.intTC = DISABLE;
    dma.intErr = DISABLE;
    dma.linkedList = (uint32_t) &lli;

    LPC_GPDMACH0 -> DMACCDestAddr = &(LPC_GPIO0 -> FIOPIN);

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(0);
}

void initTimer(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 0;

    TIM_MATCHCFG_T mat00;
    mat00.channel = 0;
    mat00.intEn = DISABLE;
    mat00.stopEn = DISABLE;
    mat00.resetEn = ENABLE;
    mat00.extOpt = TIM_TOGGLE;
    mat00.matchValue = 500; //500us = 0,5ms

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat00);
    TIM_Enable(LPC_TIM0);
}

int main(void)
{
    initPines();
    initWave();
    initTimer();
    initGPDMA();

    while(1)
    {

    }
}