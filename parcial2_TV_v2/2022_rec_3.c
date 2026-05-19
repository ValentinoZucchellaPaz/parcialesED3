#include "LPC17xx.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_timer.h"

uint32_t* scrDir = 0x10000800; //2048 bytes de datos
uint32_t* dstDir = 0x10002800;


void initTimer1(void)
{
    TIM_TIMERCFG_T tim1;
    tim1.prescaleOpt = TIM_US;
    tim1.prescaleValue = 1000; //1ms

    TIM_MATCHCFG_T mat10;
    mat10.channel = 0;
    mat10.intEn = DISABLE;
    mat10.stopEn = DISABLE;
    mat10.resetEn = ENABLE;
    mat10.extOpt = TIM_NOTHING; //es mas recomendable poner toggle
    mat10.matchValue = 1; //una transferencia cada ms

    TIM_InitTimer(LPC_TIM1, &tim1);
    TIM_ConfigMatch(LPC_TIM1, &mat10);
    TIM_Enable(LPC_TIM1);
}

void initGPDMA(void)
{
    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 7;
    dma.transferSize = 1024; //si transferimos en 2 bytes entonces hacemos 2048/2
    dma.type = GPDMA_M2M;
    dma.scrMemAddr = (uint32_t) scrDir;
    dma.dstMemAddr = (uint32_t) dstDir;
    dma.scrConn = GPDMA_MAT1_0;
    dma.dstConn = 0;
    dma.scr.width = GPDMA_HALFWORD;
    dma.scr.burst = GPDMA_BSIZE_1;
    dma.scr.increment = ENABLE;
    dma.dst.width = GPDMA_HALFWORD;
    dma.dst.burst = GPDMA_BSIZE_1;
    dma.dst.increment = ENABLE;
    dma.intTC = DISABLE;
    dma.intErr = DISABLE;
    dma.linkedList = 0;
    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(7);
}