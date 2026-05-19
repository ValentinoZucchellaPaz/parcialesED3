#include "LPC17xx.h"
#include "LPC17xx_timer.h"
#include "LPC17xx_dac.h"
#include "LPC17xx_pinsel.h"

volatile uint32_t = muestras[10] = {0};
uint32_t tiempo_actual = 0;
uint32_t tiempo_alto = 0;
uint32_t ultimo_alto = 0;
uint32_t periodo = 0;
uint8_t nivel = 0;
uint8_t indice = 0;
uint32_t promedio = 0;
uint32_t datoDAC;

void initPin(void)
{
    PINSEL_CFG_T pin;
    pin.port = 0;
    pin.pin = 26;
    pin.func = PINSEL_FUNC_00;
    pin.mode = PINSEL_TRISTATE;
    pin.openDrain = DISABLE;

    PINSEL_ConfigPin(&pin);
}

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 1000; //1ms;

    TIM_CAPTURECFG_T cap;
    cap.channel = 0;
    cap.risingEn = ENABLE;
    cap.fallingEn = ENABLE;
    cap.intEn = ENABLE;

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigCapture(LPC_TIM0, &cap);
    TIM_PinConfig(TIM_CAP0_0_P1_26);
    TIM_Enable(LPC_TIM0);
}

void TIMER0_IRQHandler(void)
{
    if(TIM_GetIntStatus(LPC_TIM0, TIM_CR0_INT))
    {
        tiempo_actual = TIM_GetCaptureValue(LPC_TIM0, 0);
        
        if(nivel == 0)
        {
            nivel = 1;
            periodo = tiempo_actual - ultimo_alto;
            muestras[indice] = (tiempo_alto*100)/periodo;
            ultimo_alto = tiempo_actual;

            uint32_t sumatoria = 0;

            for(int i = 0; i < 10; i++)
            {
                sumatoria += muestras[i];
            }

            promedio = sumatoria/10;
            datoDAC = (promedio * 620/100) << 6;
        }
        else if(nivel == 1)
        {
            nivel = 0;
            tiempo_alto = tiempo_actual - ultimo_alto;
        }

        TIM_ClearIntPending(LPC_TIM0, TIM_CR0_INT);
    }
}

void initDAC(void)
{
    DAC_CONVERTER_CFG_T dac;
    dac.doubleBuffer = DISABLE;
    dac.dmaCounter = ENABLE;
    dac.dmaRequest = ENABLE;

    DAC_Init();
    DAC_ConverterDAConverterControl(&dac);
    DAC_SetDMATimeOut(12500000); //suponiendo pclk de 25mhz
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) &datoDAC;
    lli.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 10 | (2 << 18) | (2 << 21) | (1 << 26);

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 10;
    dma.type = GPDMA_M2P;
    dma.scrMemAddr = (uint32_t) &datoDAC;
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
    initTimer0();
    initDAC();
    initGPDMA();

    while(1)
    {

    }
}

