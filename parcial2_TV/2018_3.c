#include "LPC17xx.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_timer.h"

uint32_t* src = (uint32_t*) 0x10000800; //2048 bytes de datos
uint32_t* dst = (uint32_t*) 0x10002800;

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 0;

    TIM_MATCHCFG_T mat00;
    mat00.channel = 0;
    mat00.intEn = DISABLE;
    mat00.stopEn = DISABLE;
    mat00.resetEn = ENABLE;
    mat00.extOpt = TIM_NOTHING;
    mat00.matchValue = 100; //una transf. cada 100 us;

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat00);
    TIM_Enable(LPC_TIM0)
}

void initGPDMA(void)
{
    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 1024;
    dma.type = GPDMA_M2P;
    dma.scrMemAddr = (uint32_t) src;
    dma.dstMemAddr = 0;
    dma.scrConn = 0;
    dma.dstConn = GPDMA_MAT0_0;
    dma.scr.width = GPDMA_HALFWORD;
    dma.scr.burst = GPDMA_BSIZE1;
    dma.scr.increment = ENABLE;
    dma.dst.width = GPDMA_HALFWORD;
    dma.dst.burst = GPDMA_BSIZE_1;
    dma.dst.increment = ENABLE;
    dma.intTC = DISABLE;
    dma.intErr = DISABLE;
    dma.linkedList = 0;

    LPC_GPDMACH0->DMACCDestAddr = 0x10002800;

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(0);
}

int main(void)
{
    initTimer0();
    initGPDMA();

    while(1)
    {

    }
}