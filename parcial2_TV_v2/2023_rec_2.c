#include "LPC17xx.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"

volatile uint8_t num_interrupciones = 0;
GPDMA_LLI_T lli0, lli1, lli0x, lli2x;

void initLLIs(void)
{
    //GPDMA_LLI_T lli0; //int 1
    lli0.scrAddr = (uint32_t) DIRECCION_BLOQUE_0;
    lli0.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli0.nextLLI = (uint32_t) &lli0;
    lli0.control = 1024 | (2 << 18) | (2 << 21) | (1 << 26); //tamaño: 4096 bytes en viajes de a 4 bytes son 1024, increment en src

    //GPDMA_LLI_T lli1; //int 2
    lli1.scrAddr = (uint32_t) DIRECCION_BLOQUE_1;
    lli1.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli1.nextLLI = (uint32_t) &lli1;
    lli1.control = 1024 | (2 << 18) | (2 << 21) | (1 << 26);

    //GPDMA_LLI_T lli0x; //int 3
    lli0x.scrAddr = (uint32_t) DIRECCION_BLOQUE_0;
    lli0x.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli0x.nextLLI = (uint32_t) &lli2x;
    lli0x.control = 1024 | (2 << 18) | (2 << 21) | (1 << 26);

    //GPDMA_LLI_T lli2x; //int 3
    lli2x.scrAddr = (uint32_t) DIRECCION_BLOQUE_2;
    lli2x.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli2x.nextLLI = (uint32_t) &lli0x;
    lli2x.control = 1024 | (2 << 18) | (2 << 21) | (1 << 26); 
}

void initDMA(GPDMA_LLI_T lli, uint32_t dir)
{
    GPDMA_ChannelStop(0);
    
    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 1024;
    dma.type = GPDMA_M2P;
    dma.scrMemAddr =  dir;
    dma.dstMemAddr = 0;
    dma.scrConn = 0;
    dma.dstConn = GPDMA_DAC;
    dma.src.width = GPDMA_WORD;
    dma.scr.burst = GPDMA_BSIZE_1;
    dma.scr.increment = ENABLE;
    dma.dst.width = GPDMA_WORD;
    dma.dst.burst = GPDMA_BSIZE_1;
    dma.dst.increment = DISABLE;
    dma.intTC = DISABLE;
    dma.inrErr = DISABLE;
    dma.linkedList = (uint32_t) &lli;

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(0);
}

void initDAC(uint16_t t)
{
    DAC_CONVERTER_CFG_T dac;
    dac.doubleBuffer = DISABLE;
    dac.dmaCounter = ENABLE;
    dac.dmaRequest = ENABLE;

    if(t <= 62)
    {
        DAC_SetBias(DAC_700uA);
    }
    else
    {
        DAC_SetBias(DAC_350uA);
    }

    DAC_Init();
    DAC_ConfigDAConverterControl(&dac);
    DAC_SetDMATimeOut(t);//asumiendo pclk de 25mhz
}

void initPin(void)
{
    PINSEL_CFG_T pin;
    pin.port = 2;
    pin.pin = 10;
    pin.func = PINSEL_FUNC_00;
    pin.mode = PINSEL_PULLUP;
    pin.openDrain = DISABLE;

    PINSEL_ConfigPin(&pin);

    GPIO_SetDir(2, (1 << 10), GPIO_INPUT);
    GPIO_IntConfigPin(2, 10, GPIO_INT_FALLING, ENABLE);
    NVIC_EnableIRQ(EINT2_IRQn);
}

void EINT2_IRQHandler(void)
{
    num_interrupciones = (num_interrupciones + 1)%4;

    switch(num_interrupciones)
    {
        case 1:
        GPDMA_ChannelGracefulStop(0);
        initDMA(&lli0, DIRECCION_BLOQUE_0);
        initDAC(417); //25mhz/60khz
        break;
        case 2:
        GPDMA_ChannelGracefulStop(0);
        initDMA(&lli1, DIRECCION_BLOQUE_1);
        initDAC(208); //25mhz/60khz
        break;
        case 3:
        GPDMA_ChannelGracefulStop(0);
        initDMA(&lli0x, DIRECCION_BLOQUE_0);
        initDAC(56); //25mhz/450khz
        break;
        default:
        break;
    }

    GPIO_ClearInt(2, (1 << 10));
}

int main(void)
{
    initLLIs();
    initPin();

    while(1);
}