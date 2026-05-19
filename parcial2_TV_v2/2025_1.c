#include "LPC17xx.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"

uint32_t* source = 0x20080000; //inicio banco 1
uint32_t* destiny = 0x2007E000; //mitad banco 0

void initGPDMA(void)
{
    /*GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) source;
    lli.dstAddr = (uint32_t) destiny;
    lli.nextLLi = (uint32_t) &lli;
    lli.control = 4096 | (1 << 18) | (1 << 21) | (1 << 26) | (1 << 27);
    //8192 bytes en viajes de 2 bytes son 4096, increment en src y dst*/

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 7;
    dma.transferSize = 4095; //8192 bytes transferidos en total, se pierde un dato ya que el limite es 4095 y yo tengo 4096 viajes
    //blasco dijo q con aclarar que se pierde un dato esta bien
    dma.type = GPDMA_M2M;
    dma.scrMemAddr = (uint32_t) source;
    dma.dstMemAddr = (uint32_t) destiny;
    dma.scrConn = 0;
    dma.dstConn = 0;
    dma.scr.width = GPDMA_HALFWORD;
    dma.src.burst = GPDMA_BSIZE_4; //como hacemos m2m me puedo permitir usar un burst mas grande que 1
    dma.src.increment = ENABLE;
    dma.dst.width = GPDMA_HALFWORD; //podria ser word? preguntar (puede ser pero es mejor que sea halfword)
    dma.dst.burst = GPDMA_BSIZE_4; //preguntar (esta bien)
    dma.dst.increment = ENABLE;
    dma.intTC = ENABLE;
    dma.intErr = DISABLE;
    dma.linkedList = 0; //solo debo transferir una vez

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(7);
    NVIC_EnableIRQ(DMA_IRQn);
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

void DMA_IRQHandler(void)
{
    GPIO_SetPins(0, (1 << 22));
    GPDMA_ClearIntPending(GPDMA_CLR_INTTC, 7);
}