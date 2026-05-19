#include "LPC17xx.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_pinsel.h"

uint32_t* scrWave = 0x20080000; //382 datos a mover

void initPin(void)
{
    PINSEL_CFG_T pindac;
    pindac.port = 0;
    pindac.pin = 26;
    pindac.fun = PINSEL_FUNC_10;
    pindac.mode = PINSEL_TRISTATE;
    pindac.openDrain = DISABLE;
    PINSEL_ConfigPin(&pindac);
}

void initDAC(void)
{
    DAC_CONVERTER_CFG_T dac;
    dac.doubleBuffer = DISABLE;
    dac.dmaCounter = ENABLE;
    dac.dmaRequest = ENABLE;

    DAC_Init();
    DAC_ConfigDAConverterControl(&dac);
    DAC_SetDMATimeOut(65445); //382 muestras por segundo, suponiendo 25mhz de pclk, 1/382 * 25000000
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) scrWave;
    lli.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 382 | (2 << 18) | (2 << 21) | (1 << 26);

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 382;
    dma.type = GPDMA_M2P;
    dma.scrMemAddr = (uint32_t) scrWave;
    dma.dstMemAddr = 0;
    dma.scrConn = 0;
    dma.dstConn = GPDMA_DAC;
    dma.scr.width = GPDMA_WORD;
    dma.scr.burst = GPDMA_BSIZE_1;
    dma.scr.increment = ENABLE;
    dma.dst.width = GPDMA_WORD;
    dma.dst.burst = GPDMA_BSIZE_1;
    dma.dst.increment = DISABLE;
    dma.intTC = DISABLE;
    dma.intErr = DISABLE;
    dma.linkedList = (uint32_t) &lli;

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(0);
}

int main(void)
{
    initPin();
    initDAC();
    initGPDMA();

    while(1)
    {
        
    }
}
