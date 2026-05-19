#include "LPC17xx.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_timer.h"

uint32_t* srcdir = 0x10000800;
uint32_t* dstdir = 0x10002800;

void initTimer1(void)
{
    TIM_TIMERCFG_T tim1;
    tim1.prescaleOpt = TIM_US;
    tim1.prescaleValue = 1000; //1 dato cada ms

    TIM_MATCHCFG_T mat10;
    mat10.channel = 0;
    mat10.intEn = DISABLE;
    mat10.stopEn = DISABLE;
    mat10.resetEn = ENABLE;
    mat10.extOpt = TIM_TOGGLE;
    mat10.matchValue = 1;

    TIM_InitTimer(LPC_TIM1, &tim1);
    TIM_ConfigMatch(LPC_TIM1, &mat10);
    TIM_Cmd(LPC_TIM1, ENABLE);
}

void initGPDMA(void)
{
    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 7;
    dma.transferSize = 1024; //2048 bytes transferidos en 16 bits son 1024 viajes
    dma.type = GPDMA_M2M;
    dma.scrMemAddr = (uint32_t) scrdir;
    dma.dstMemAddr = (uint32_t) dstdir;
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

int main(void)
{
    initTimer1();
    initGPDMA();

    while(1)
    {

    }
}