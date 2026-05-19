#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_timer.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_exti.h"

uint32_t* datosADC = (uint32_t*) 0x2007C000;
uint32_t* datosWave = (uint32_t*) 0x2007E000;
uint8_t estado = 0;

//falta la generacion de onda

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 0;

    TIM_MATCHCFG_T mat01;
    mat01.channel = 1;
    mat01.intEn = DISABLE;
    mat01.stopEn = DISABLE;
    mat01.resetEn = ENABLE;
    mat01.extOpt = TIM_TOGGLE;
    mat01.matchValue = 15; //30us para 32khz, solo tomo los flancos de bajada asi que uso 15us

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initADC(void)
{
    ADC_Init(200000);
    ADC_PinConfig(0);
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_ChannelEnable(0);
    ADC_EdgeStartConfig(ADC_START_ON_FALLING);
    ADC_PowerUp();
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli0;
    lli0.scrAddr = (uint32_t) &(LPC_ADC -> ADDR0);
    lli0.dstAddr = (uint32_t) datosADC;
    lli0.nextLLI = (uint32_t) &lli;
    lli0.control = 2048 | (2 << 18) | (2 << 21) | (1 << 27);

    GPDMA_Channel_CFG_T dma0;
    dma0.channelNum = 0;
    dma0.transferSize = 2048;
    dma0.type = GPDMA_P2M;
    dma0.scrMemAddr = 0;
    dma0.dstMemAddr = (uint32_t) datosADC;
    dma0.scrConn = GPDMA_ADC;
    dma0.dstConn = 0;
    dma0.src.width = GPDMA_WORD;
    dma0.src.burst = GPDMA_BSIZE_1;
    dma0.src.increment = DISABLE;
    dma0.dst.width = GPDMA_WORD;
    dma0.dst.burst = GPDMA_BSIZE_1;
    dma0.dst.increment = ENABLE;
    dma0.intTC = DISABLE;
    dma0.intErr = DISABLE;
    dma0.linkedList = (uint32_t) &lli0;

    GPDMA_LLI_T lli1;
    lli1.scrAddr = (uint32_t) datosWave;
    lli1.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli1.nextLLI = (uint32_t) &lli1;
    lli1.control = 2048 | (2 << 18) | (2 << 21) | (1 << 26);

    GPDMA_Channel_CFG_T dma1;
    dma1.channelNum = 1;
    dma1.transferSize = 2048;
    dma1.type = GPDMA_M2P;
    dma1.scrMemAddr = (uint32_t) datosWave;
    dma1.dstMemAddr = 0;
    dma1.scrConn = 0;
    dma1.dstConn = GPDMA_DAC;
    dma1.src.width = GPDMA_WORD;
    dma1.src.burst = GPDMA_BSIZE_1;
    dma1.src.increment = ENABLE;
    dma1.dst.width = GPDMA_WORD;
    dma1.dst.burst = GPDMA_BSIZE_1;
    dma1.dst.inrement = DISABLE;
    dma1.intTC = DISABLE;
    dma1.intErr = DISABLE;
    dma1.linkedList = (uint32_t) lli1;

    GPDMA_LLI_T lli2;
    lli2.scrMemAddr = (uint32_t) datosADC;
    lli2.dstMemAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli2.nextLLI (uint32_t) &lli2;
    lli2.control = 2048 | (2 << 18) | (2 << 21) | (1 << 26);

    GPDMA_Channel_CFG_T dma2 = {
    .channelNum = 2,
    .transferSize = 2048,
    .type = GPDMA_M2P,
    .srcMemAddr = (uint32_t) datosADC,
    .dstMemAddr = 0,
    .srcConn = 0,
    .dstConn = GPDMA_DAC,
    .src = {GPDMA_WORD, GPDMA_BSIZE_1, ENABLE},  // Aquí SÍ funciona
    .dst = {GPDMA_WORD, GPDMA_BSIZE_1, DISABLE}, // Aquí SÍ funciona
    .intTC = DISABLE,
    .intErr = DISABLE,
    .linkedList = (uint32_t) &lli2
    };

    GPDMA_Init();
    GPDMA_SetupChannel(&dma0);
    GPDMA_SetupChannel(&dma1);
    GPDMA_SetupChannel(&dma2);
    GPDMA_ChannelStart(0);
    GPDMA_ChannelStart(1);
}

void initDAC(void)
{
    DAC_CONVERTER_CFG_T dac;
    dac.doubleBuffer = DISABLE;
    dac.dmaCounter = ENABLE;
    dac.dmaRequest = ENABLE;

    DAC_Init();
    DAC_ConfigDAConverterControl(&dac);
    DAC_SetDMATimeOut(6); //para hacer 2048 viajes en 614us necesito un viaje por 0,3us, suponemos pclk a 20mhz
}

void initPines(void)
{
    EXTI_CFG_T eint0 = {EXTI_EINT0, EXTI_LEDGE_SENSITIVE, EXTI_FALLING_EDGE};
    EXTI_Init();
    EXTI_PinConfig(EXTI_EINT0, EXTI_PULLUP);
    EXTI_ConfigEnable(&eint0);

    PINSEL_CFG_T pinDac;
    pinDac.port = 0;
    pinDac.pin = 26;
    pinDac.func = PINSEL_FUNC_10;
    pinDac.mode = PINSEL_TRISTATE;
    pinDac.openDrain = DISABLE;
    PINSEL_ConfigPin(&pinDac);
}

void EINT0_IRQHandlder(void)
{
    if(estado == 0)
    {
        while(!ADC_ChannelGetStatus(0, ADC_DATA_DONE)) {}
        GPDMA_ChannelGracefulStop(0);
        GPDMA_ChannelGracefulStop(1);
        GPDMA_ChannelStart(2);
        estado = 1;
    }
    else if (estado == 1)
    {
        GPDMA_ChannelGracefulStop(2);
        GPDMA_ChannelStart(0);
        GPDMA_ChannelStart(1);
        estado = 0;
    }

    EXTI_ClearFlag(EXTI_EINT0);
}

int main(void)
{
    initTimer0();
    initADC();
    initGPDMA();
    initDAC();
    initPines();
    
    while(1)
    {

    }
}
