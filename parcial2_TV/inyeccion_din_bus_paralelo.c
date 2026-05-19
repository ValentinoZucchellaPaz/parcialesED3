#include "LPC17xx.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"
#include "LPC17xx_gpdma.h"

uint32_t* src0 = 0x2007C000;
uint32_t* scr1 = 0x2007C200;

void initPines(void)
{
    PINSEL_CFG_T p0;
    p0.port = 0;
    p0.func = PINSEL_FUNC_00;
    p0.mode = PINSEL_TRISTATE;
    p0.openDrain = DISABLE;

    PINSEL_CFG_T p1;
    p1.port = 1;
    p1.func = PINSEL_FUNC_00;
    p1.mode = PINSEL_TRISTATE;
    p1.openDrain = DISABLE;

    PINSEL_ConfigMultiplePins(&p0, 0xFF);
    PINSEL_ConfigMultiplePins(&p1, 0xFF);
    GPIO_SetDir(0, 0xFF, GPIO_OUTPUT);
    GPIO_SetDir(1, 0xFF, GPIO_OUTPUT);

    PINSEL_CFG_T p20;
    p20.port = 2;
    p20.pin = 0;
    p20.func = PINSEL_FUNC_00;
    p20.mode = PINSEL_PULLUP;
    p20.openDrain = DISABLE;
    PINSEL_ConfigPin(&p20);

    PINSEL_CFG_T p21;
    p21.port = 2;
    p21.pin = 1;
    p21.func = PINSEL_FUNC_00;
    p21.mode = PINSEL_PULLUP;
    p21.openDrain = DISABLE;
    PINSEL_ConfigPin(&p21);

    GPIO_SetDir(2, 3, GPIO_INPUT);
    GPIO_IntConfigPort(2, 3, GPIO_INT_FALLING);
    NVIC_EnableIRQ(EINT3_IRQn);
}

void initGPDMA(void)
{
    GPDMA_Channel_CFG_T dma0;
    dma0.channelNum = 7;
    dma0.transferSize = 128; //512 bytes en viajes de a 4
    dma0.type = GPDMA_M2M;
    dma0.scrMemAddr = (uint32_t) src0;
    dma0.dstMemAddr = (uint32_t) &(LPC_GPIO0 -> FIOPIN);
    dma0.scrConn = 0;
    dma0.dstConn = 0;
    dma0.scr.width = GPDMA_WORD;
    dma0.scr.burst = GPDMA_BSIZE_1;
    dma0.scr.increment = ENABLE;
    dma0.dst.width = GPDMA_WORD;
    dma0.dst.burst = GPDMA_BSIZE_1;
    dma0.dst.increment = DISABLE;
    dma0.intTC = DISABLE;
    dma0.intErr = DISABLE;
    dma0.linkedList = 0;

    GPDMA_Channel_CFG_T dma1;
    dma1.channelNum = 6;
    dma1.transferSize = 128; //512 bytes en viajes de a 4
    dma1.type = GPDMA_M2M;
    dma1.scrMemAddr = (uint32_t) src1;
    dma1.dstMemAddr = (uint32_t) &(LPC_GPIO1 -> FIOPIN);
    dma1.scrConn = 0;
    dma1.dstConn = 0;
    dma1.scr.width = GPDMA_WORD;
    dma1.scr.burst = GPDMA_BSIZE_1;
    dma1.scr.increment = ENABLE;
    dma1.dst.width = GPDMA_WORD;
    dma1.dst.burst = GPDMA_BSIZE_1;
    dma1.dst.increment = DISABLE;
    dma1.intTC = DISABLE;
    dma1.intErr = DISABLE;
    dma1.linkedList = 0;

    GPDMA_Init();
    GPDMA_SetupChannel(&dma0);
    GPDMA_SetupChannel(&dma1);
}

void EINT3_IRQHandler(void)
{
    if(GPIO_GetPinIntStatus(2, 0, GPIO_INT_FALLING))
    {
        GPDMA_ChannelStart(7);
        GPIO_ClearInt(2, (1 << 0));
    }
    else if(GPIO_GetPinIntStatus(2, 1, GPIO_INT_FALLING))
    {
        GPDMA_ChannelStart(6);
        GPIO_ClearInt(2, (1 << 1));
    }
}

int main(void)
{
    initPines();
    initGPDMA();

    while(1)
    {

    }
}
