#include "LPC17xx.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"

uint32_t* src = (uint32_t*) 0x20080000; //8192 bytes
uint32_t* dst = (uint32_t*) 0x2007E000;

void initGPDMA(void)
{
    GPDMA_Channel_CFG_T dma = {
        .channelNum = 7,
        .transferSize = 4096,
        .type = GPDMA_M2M,
        .srcMemAddr = (uint32_t) src,
        .dstMemAddr = (uint32_t) dst,
        .scrConn = 0,
        .dstConn = 0,
        .src = {GPDMA_HALFWORD, GPDMA_BSIZE_4, ENABLE},
        .dst = {GPDMA_HALFWORD, GPDMA_BSIZE_4, ENABLE},
        .intTC = ENABLE,
        .intErr = DISABLE,
        .linkedList = 0
    };

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    NVIC_EnableIRQ(DMA_IRQn);
    GPDMA_ChannelStart(7);
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
    if(GPDMA_IntGetStatus(GPDMA_INTTC, 7))
    {
        GPIO_SetPinState(0, 22, SET);
        GPDMA_ClearIntPending(GPDMA_INTTC, 7);
    }
}

int main(void)
{
    initGPDMA();
    initPin();

    while(1)
    {

    }
}