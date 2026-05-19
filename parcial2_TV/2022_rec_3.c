#include "LPC17xx.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_timer.h"

uint32_t* src = (uint32_t*) 0x10000800; //2048 bytes a mover
uint32_t* dst = (uint32_t*) 0x10002800;

void initTimer1(void)
{
    TIM_TIMERCFG_T tim1;
    tim1.prescaleOpt = TIM_US;
    tim1.prescaleValue = 0;
    
    TIM_MATCHCFG_T mat10;
    mat10.channel = 0;
    mat10.intEn = DISABLE;
    mat10.stopEn = DISABLE;
    mat10.resetEn = ENABLE;
    mat10.extOpt = TIM_NOTHING;
    mat10.matchValue = 100; //10us por dato

    TIM_InitTimer(LPC_TIM1);
    TIM_ConfigMatch(LPC_TIM1, &mat10):
    TIM_Enable(LPC_TIM1);
}

void initGPDMA(void)
{
    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 7;
    dma.transferSize = 1024; //2048 bytes en viajes de 2 bytes
    dma.type = GPDMA_M2P; //m2p para poder sincronizar por timer
    dma.scrMemAddr = (uint32_t) src;
    dma.dstMemAddr = 0;
    dma.scrConn = 0;
    dma.dstConn = GPDMA_MAT1_0;
    dma.src.width = GPDMA_HALFWORD;
    dma.src.burst = GPDMA_BSIZE_1;
    dma.scr.increment = ENABLE;
    dma.dst.width = GPDMA_HALFWORD;
    dma.dst.burst = GPDMA_BSIZE_1;
    dma.dst.increment = ENABLE;
    dma.intTC = DISABLE;
    dma.intErr = DISABLE;
    dma.linkedList = 0;

    LPC_GPDMACH0 -> DMACCDestAddr = dst;

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(7);
}

int main(void)
{
    initTimer1();
    initGPDMA();

    while(1)
    {

    }
}