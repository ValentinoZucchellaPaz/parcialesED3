#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_timer.h"

void initADC(void)
{
    ADC_Init(400000); //con un canal usamos 200khz, con 2 usamos 400khz
    ADC_PowerUp();
    ADC_PinConfig(2);
    ADC_PinConfig(4);
    ADC_StartCmd(ADC_START_ON_MAT0_0);
    ADC_EdgeStartConfig(ADC_START_ON_RISING);
}

volatile uint32_t datosCanal2[20] = {0};
volatile uint32_t datosCanal4[20] = {0};

void initGPDMA(void)
{
    GPDMA_LLI_T lli2;
    lli2.scrAddr = (uint32_t) &(LPC_ADC -> ADDR2);
    lli2.dstAddr = (uint32_t) &(datosCanal2);
    lli2.nextLLI = (uint32_t) &lli4;
    lli2.control = 20 | (2 << 18) | (2 << 21) | (1 << 27); //32 bits de ancho, increment en dst, 20 datos movidos antes de ciclar

    GPDMA_LLI_T lli4;
    lli4.scrAddr = (uint32_t) &(LPC_ADC -> ADDR4);
    lli4.dstAddr = (uint32_t) &(datosCanal4);
    lli4.nextLLI = (uint32_t) &lli2;
    lli4.control = 20 | (2 << 18) | (2 << 21) | (1 << 27); //32 bits de ancho, increment en dst, 20 datos movidos antes de ciclar

    GPDMA_Channel_CFG_T dma2;
    dma2.channelNum = 0;
    dma2.transferSize = 20;
    dma2.type = GPDMA_P2M;
    dma2.scrMemAddr = 0; //deberia especificar el canal aca?
    dma2.dstMemAddr = (uint32_t) &(datosCanal2);
    dma2.scrConn = GPDMA_ADC; //duda aca, no puedo especificar canal
    dma2.scr.width = GPDMA_WORD;
    dma2.scr.burst = GPDMA_BSIZE_1;
    dma2.scr.increment = DISABLE;
    dma2.dst.width = GPDMA_HALF_WORD
    dma2.dst.burst = GPDMA_BSIZE_1;
    dma2.dst.increment = ENABLE;
    dma2.intTC = DISABLE;
    dma2.intErr = DISABLE;
    dma2.linkedList = (uint32_t) &lli2;

    /*
    GPDMA_Channel_CFG_T dma4;
    dma4.channelNum = 0;
    dma4.transferSize = 20;
    dma4.type = GPDMA_P2M;
    dma4.scrMemAddr = 0;
    dma4.dstMemAddr = (uint32_t) &(datosCanal4);
    dma4.scrConn = GPDMA_ADC;
    dma4.scr.width = GPDMA_WORD;
    dma4.scr.burst = GPDMA_BSIZE_1;
    dma4.scr.increment = DISABLE;
    dma4.dst.width = GPDMA_WORD
    dma4.dst.burst = GPDMA_BSIZE_1;
    dma4.dst.increment = ENABLE;
    dma4.intTC = DISABLE;
    dma4.intErr = DISABLE;
    dma4.linkedList = (uint32_t) &lli4;
    */

    GPDMA_Init();
    GPDMA_SetupChannel(&dma2);
    //GPDMA_SetupChannel(&dma4);
    GPDMA_ChannelStart(0);
    //GPMDA_ChannelStart(4);
}

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_TICK;
    tim0.prescaleValue = 0;

    TIM_MATCHCFG_T mat00;
    mat00.channel = 0;
    mat00.intEn = ENABLE;
    mat00.stopEn = DISABLE;
    mat00.resetEn = ENABLE;
    mat00.extOpt = TIM_TOGGLE;
    mat00.matchValue = 5000; //100mhz/20khz

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat00);
    NVIC_EnableIRQ(TIMER0_IRQn);
    TIM_Enable(LPC_TIM0);
}

void TIMER0_IRQHandler(void) {
    static uint8_t alternar = 0;
    
    if (alternar == 0) {
        ADC_ChannelDisable(ADC_CHANNEL_4);
        ADC_ChannelEnable(ADC_CHANNEL_2);
        alternar = 1;
    } else {
        ADC_ChannelDisable(ADC_CHANNEL_2);
        ADC_ChannelEnable(ADC_CHANNEL_4);
        alternar = 0;
    }
    
    TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}

/*anotacion de revision: podria haber configurado los 2 dma, uno para cada canal de adc y prender ambos adc a la vez con el 
mat01, era mas facil

CAMBIE EL DST width a 16 porque es al pedo guardar la 2da mitad del registro adc*/