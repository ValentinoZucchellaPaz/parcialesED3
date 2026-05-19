#include "LPC17xx.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"
#include "LPC17xx_exti.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_gpdma.h"

uint32_t* dst = 0x2007C000; //usamos la mitad del banco para adc y la otra para promedios
volatile uint8_t estado = 0;
volatile uint32_t indice = 0;
volatile uint32_t indiceP = 0;
uint32_t promedio = 0;
uint32_t* dstP = 0x2007E000;

void initPin(void)
{
    PINSEL_CFG_T pin;
    pin.port = 2;
    pin.pin = 10;
    pin.func = PINSEL_FUNC_00;
    pin.mode = PINSEL_PULLDOWN;
    pin.openDrain = DISABLE;

    PINSEL_ConfigPin(&pin);
    GPIO_SetDir(2, (1 << 10), GPIO_INPUT);

    EXTI_CFG_T eint0;
    eint0.line = EXTI_EINT0;
    eint0.mode = EXTI_LEDGE_SENSITIVE;
    eint0.polarity = EXTI_RISING_EDGE;

    EXTI_Init();
    EXTI_Config(&eint0);
    EINT_EnableIRQ(EINT0_IRQn);
}

void initADC(void)
{
    ADC_Init(200000);
    ADC_PinConfig(0);
    ADC_PowerUp();
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_ChannelEnable(0);
    ADC_EdgeStartConfig(ADC_START_ON_RISING);
    ADC_IntEnable(ADC_INT_CH0);
    NVIC_EnableIRQ(ADC_IRQn);
}

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
    mat01.matchValue = 3; //mitad de 6 para hacer 160khz de muestreo (solo tomamos el rising)

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) &(LPC_ADC -> ADDR0);
    lli.dstAddr = (uint32_t) dst1;
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 2048 | (2 << 18) | (2 << 21) | (1 << 27) | (1 << 31);

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 2048; //creo q me estoy sobrepasando del size maximo
    dma.type = GPDMA_P2M;
    dma.scrMemAddr = 0;
    dma.dstMemAddr = (uint32_t) dst1;
    dma.scrConn = GPDMA_ADC;
    dma.dstConn = 0;
    dma.scr.width = GPDMA_WORD;
    dma.scr.burst = GPDMA_BSIZE_1;
    dma.scr.increment = DISABLE;
    dma.dst.width = GPDMA_WORD;
    dma.dst.burst = GPDMA_BSIZE_1;
    dma.dst.increment = ENABLE;
    dma.intTC = DISABLE;
    dma.intErr = DISABLE;
    dma.linkedList = (uint32_t) &lli;

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    NVIC_EnableIRQ(DMA_IRQn);
    GPDMA_ChannelStart(0);
}

void DMA_IRQHandler(void)
{
    promediarADC();
    GPDMA_ClearIntPending(GPDMA_CLR_INTTC, 0);
}

void promediarADC(void)
{
    uint32_t sumatoria;

    for(uint32_t i = 0; i < 2048; i++)
    {
        sumatoria += (dst[i] >> 4)&0xFFF;
    }

    promedio = sumatoria/2048;

    dstP[indiceP] = promedio;
    indice ++;
    if(indice >= 2048) 
    {
        indiceP = 0;
    }

}

void EINT0_IRQHandler(void)
{
    if(EXTI_GetFlag(EXTI_EINT0))
    {
        if(estado == 0)
        {
            estado == 1
            ADC_IntDisable(ADC_INT_CH0);
            GPDMA_ChannelStart(0);
        }
        else if(estado == 1)
        {
            estado == 0;
            GPDMA_ChannelGracefulStop(0);
            ADC_IntEnable(ADC_INT_CH0);
        }

        EXTI_ClearFlag(EXTI_EINT0);
    }
}

int main(void)
{
    initPin();
    initADC();
    initTimer0();
    initGPDMA();
    
    while(1)
    {

    }
}

/*
    un canal dma pasa los datos de adc a la primera mitad del primer banco, un timer1, cada 1seg, genera una interrupcion que
    revisa el estado y, dependiendo de este, usa un ciclo for y punteros para llevar los datos al banco 0 o habilita un dma para que pase 
    los datos al banco 0.
    Una vez se hayan pasado los datos, promediarlos y moverlos a la 2da mitad del banco 0 con un for y con punteros

    QUE EJERCICIO DE MIERDA
*/