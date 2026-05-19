#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"

uint32_t* datosADC = 0x2007C000; //0x2007FFFF
uint32* datosWave = 0x2007E000; // 8192 bytes de datos
volatile uint8_t estadoInt = 0; 

void initADC(void)
{
    ADC_Init(200000);
    ADC_PinConfig(0);
    ADC_PowerUp();
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_ChannelEnable(0);
}

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 1;

    TIM_MATCHCFG_T mat01;
    mat01.channel = 1;
    mat01.intEn = DISABLE;
    mat01.stopEn = DISABLE;
    mat01.resetEn = ENABLE;
    mat01.extOpt = TIM_TOGGLE;
    mat01.matchValue = 30; //32us para 32khz, pongo 30 para llegar mas seguro

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initGPDMA(void)
{
    static GPDMA_LLI_T lli0;
    lli0.scrAddr = (uint32_t) &(LPC_ADC->ADDR0);
    lli0.dstAddr = (uint32_t) datosADC;
    lli0.nextLLI = (uint32_t) &lli0;
    lli0.control = 2048 | (2 << 18) | (2 << 21) | (1 << 27) | (1 << 31); //8192 bytes en viajes de 4 bytes, increment en dst

    GPDMA_Channel_CFG_T dmaAdc;
    dmaAdc.channelNum = 0;
    dmaAdc.transferSize = 2048;
    dmaAdc.type = GPDMA_P2M;
    dmaAdc.scrMemAddr = 0;
    dmaAdc.dstMemAddr = (uint32_t) datosADC;
    dmaAdc.scrConn = GPDMA_ADC;
    dmaAdc.dstConn = 0;
    dmaAdc.scr.width = GPDMA_WORD;
    dmaAdc.scr.burst = GPMDA_BSIZE_1;
    dmaAdc.scr.increment = DISABLE;
    dmaAdc.dst.width = GPDMA_WORD;
    dmaAdc.dst.burst = GPDMA_BSIZE_1;
    dmaAdc.dst.increment = ENABLE;
    dmaAdc.intTC = ENABLE;
    dmaAdc.intErr = DISABLE;
    dmaAdc.linkedList = (uint32_t) &lli0;

    static GPDMA_LLI_T lli1;
    lli1.scrAddr = (uint32_t) datosWave;
    lli1.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli1.nextLLI = (uint32_t) &lli1;
    lli1.control = 2048 | (2 << 18) | (2 << 21) | (1 << 26);

    GPDMA_Channel_CFG_T dmaDac;
    dmaDac.channelNum = 1;
    dmaDac.transferSize = 2048;
    dmaDac.type = GPDMA_M2P;
    dmaDac.scrMemAddr = (uint32_t) datosWave;
    dmaDac.dstMemAddr = 0;
    dmaDac.scrConn = 0;
    dmaDac.dstConn = GPDMA_DAC;
    dmaDac.scr.width = GPDMA_WORD;
    dmaDac.scr.burst = GPMDA_BSIZE_1;
    dmaDac.scr.increment = ENABLE;
    dmaDac.dst.width = GPDMA_WORD;
    dmaDac.dst.burst = GPDMA_BSIZE_1;
    dmaDac.dst.increment = DISABLE;
    dmaDac.intTC = DISABLE;
    dmaDac.intErr = DISABLE;
    dmaDac.linkedList = (uint32_t) &lli1;

    GPDMA_Init();
    GPDMA_SetupChannel(&dmaAdc);
    GPDMA_SetupChannel(&dmaDac);
    GPDMA_ChannelStart(0);
    GPDMA_ChannelStart(1);
}

void DMA_IRQHandler(void) {
    // Verificamos si la interrupción es del Canal 0 (ADC) por Terminal Count
    if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 0)) {
        
        // ALINEACIÓN DE BITS:
        // Recorremos el buffer sucio y lo limpiamos para el DAC
        for (int i = 0; i < SAMPLES; i++) {
            // 1. Extraer bits [15:4] (dato de 12 bits del ADC)
            uint32_t dato = (datosADC_Raw[i] >> 4) & 0xFFF; 
            
            // 2. Convertir a 10 bits y desplazar a [15:6] para el DAC
            // ADC(12 bits) >> 2 = 10 bits. Luego << 6 para posición DACR.
            datosADC_Procesados[i] = (dato >> 2) << 6;
        }

        // Limpiamos la bandera de interrupción
        GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC, 0);
    }
}

void initDAC(void)
{
    DAC_CONVERTER_CFG dac;
    dac.doubleBuffer = DISABLE;
    dac.dmaCounter = ENABLE;
    dac.dmaRequest = ENABLE;

    DAC_Init();
    DAC_ConfigDAConverterControl(&dac);
    DAC_SetDMATimeOut(24); //614us/2048 = 0,3us - 0,3us*80mhz = 24
}

void initPin(void)
{
    PINSEL_CFG_T pin;
    pin.port = 2;
    pin.pin = 15;
    pin.func = PINSEL_FUNC_00;
    pin.mode = PINSEL_PULLUP;
    pin.openDrain = DISABLE;

    PINSEL_ConfigPin(&pin);

    GPIO_SetDir(2, (1 << 15), GPIO_INPUT);
    GPIO_IntConfigPin(2, 15, GPIO_INT_FALLING, ENABLE);
    NVIC_EnableIRQ(EINT3_IRQn);
}

void initGPDMAint(void)
{
    static GPDMA_LLI_T lli2;
    lli2.scrAddr = (uint32_t) datosADC;
    lli2.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli2.nextLLI = (uint32_t) &lli2;
    lli2.control = 2048 | (2 << 18) | (2 << 21) | (1 << 26);

    GPDMA_Channel_CFG_T dma2;
    dma2.channelNum = 2;
    dma2.transferSize = 2048;
    dma2.type = GPDMA_M2P;
    dma2.scrMemAddr = (uint32_t) (datosADC);
    dma2.dstMemAddr = 0;
    dma2.scrConn = 0;
    dma2.dstConn = GPDMA_DAC;
    dma2.scr.width = GPDMA_WORD;
    dma2.scr.burst = GPDMA_BSIZE_1;
    dma2.scr.increment = ENABLE;
    dma2.dst.width = GPDMA_WORD;
    dma2.dst.burst = GPDMA_BSIZE_1;
    dma2.dst.increment = DISABLE;
    dma2.intTC = DISABLE;
    dma2.intErr = DISABLE;
    dma2.linkedList = (uint32_t) & lli2;

    GPDMA_SetupChannel(&dma2); //esto se ejecuta despues de init_GPDMA
    //el canal se habilita despues
}

void EINT3_IRQHandler(void)
{
    if(GPIO_GetPinIntStatus(2, 15, GPIO_INT_FALLING))
    {
        if(estadoInt == 0)
        {
            GPDMA_ChannelGracefulStop(0);
            GPDMA_ChannelGracefulStop(1);
            GPDMA_ChannelEnable(2);
            estadoInt = 1;
        }
        else if (estadoInt == 1)
        {
            GPDMA_ChannelGracefulStop(2);
            GPDMA_ChannelEnable(0);
            GPDMA_ChannelEnable(1);
            estadoInt = 0;
        }

        GPIO_ClearInt(2, (1 << 15));
    }
}

int main(void)
{
    initADC();
    initTimer0();
    initGPDMA();
    initGPDMAint();
    initDAC();
    initPin();

    while(1)
    {

    }
}