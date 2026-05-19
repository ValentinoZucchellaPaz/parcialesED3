#include "LPC17xx.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_exti.h"

GPDMA_LLI_T lli0, lli1, lli0x, lli2x;
uint8_t estado = 0;

void initLLIs(void)
{
    lli0.scrAddr = (uint32_t) DIRECCION_BLOQUE_0;
    lli0.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli0.nextLLI = (uint32_t) &lli0;
    lli0.control = 1024 | (2 << 18) | (2 << 21) | (1 << 26);

    lli1.scrAddr = (uint32_t) DIRECCION_BLOQUE_1;
    lli1.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli1.nextLLI = (uint32_t) &lli1;
    lli1.control = 1024 | (2 << 18) | (2 << 21) | (1 << 26);

    lli0x.scrAddr = (uint32_t) DIRECCION_BLOQUE_0;
    lli0x.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli0x.nextLLI = (uint32_t) &lli2x;
    lli0x.control = 1024 | (2 << 18) | (2 << 21) | (1 << 26);

    lli2x.scrAddr = (uint32_t) DIRECCION_BLOQUE_2;
    lli2x.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli2x.nextLLI = (uint32_t) &lli0x;
    lli2x.control = 1024 | (2 << 18) | (2 << 21) | (1 << 26);
}

void initGPDMA(GPDMA_LLI_T* lli)
{
    GPDMA_Channel_CFG_T dma = {
        .channelNum = 0,
        .transferSize = 1024,
        .type = GPDMA_M2P,
        .scrMemAddr = (uint32_t) lli -> scrAddr,
        .dstMemAddr = 0,
        .srcConn = 0,
        .dstConn = GPDMA_DAC,
        .src = {GPDMA_WORD, GPDMA_BSIZE_1, ENABLE},
        .dst = {GPDMA_WORD, GPDMA_BSIZE_1, DISABLE},
        .intTC = DISABLE;
        .intErr = DISABLE;
        .linkedList = (uint32_t) lli
    };

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(0);
}

void initEINT0(void)
{
    EXTI_CFG_T eint0 = {EXTI_EINT0, EXTI_LEDGE_SENSITIVE, EXTI_FALLING_EDGE};
    EXTI_Init();
    EXTI_PinConfig(EXTI_EINT0, EXTI_PULLUP);
    EXTI_ConfigEnable(&eint0);
}

void EINT0_IRQHandler(void)
{
    switch(estado)
    {
        case 0:
        GPDMA_ChannelGracefulStop(0);
        initGPDMA(&lli0);
        initDAC(417);
        estado = 1;
        break;
        case 1:
        GPDMA_ChannelGracefulStop(0);
        initGPDMA(&lli1);
        initDAC(208);
        estado = 2;
        break;
        case 2:
        GPDMA_ChannelGracefulStop(0);
        initGPDMA(&lli0x);
        initDAC(56);
        estado = 0;
        break;
        default:
        break;
    }

    EXTI_ClearFlag(EXTI_EINT0);
}

void initDAC(uint32_t t)
{
    DAC_CONVERTERCFG dac = {DISABLE, ENABLE, ENABLE};
    DAC_Init();
    DAC_ConfigDAConverterControl(&dac);

    if(t <= 62) {DAC_SetBias(DAC_700uA);}
    else {DAC_SetBias(DAC_350uA);}

    DAC_SetDMATimeOut(t); //suponiendo pclk a 25mhz
}

int main(void)
{
    initLLIs();
    initEINT0();

    while(1)
    {

    }
}

