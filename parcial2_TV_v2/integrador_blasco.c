#include "LPC17xx.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpio.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_timer.h"

uint32_t* inicio = 0x2007C000; //8192 bytes (solo usamos la mitad del banco, la otra mitad se usa despues)
volatile uint8_t indice = 0;
volatile uint8_t numInt = 0;
uint32_t* inicioM2M = 0x2007E000;
uint32_t promedio;


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

    EXTI_CFG_T eint2;
    eint2.line = EXTI_EINT2;
    eint2.mode = EXTI_EDGE_SENSITIVE;
    eint2.polarity =  EXTI_RISING_EDGE;
    EXTI_Config(&eint2);

    NVIC_EnableIRQ(EINT2_IRQn);
}

void initADC(void)
{
    ADC_Init(2000000);
    ADC_PinConfig(0);
    ADC_PowerUp();
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_ChannelEnable(0);
    ADC_IntEnable(ADC_INT_CH0); //empezamos con el modo de los punteros
    NVIC_EnableIRQ(ADC_IRQn);
}

void ADC_IRQHandler(void)
{
    if(ADC_ChannelGetStatus(0, ADC_DATA_DONE))
    {
        inicio[indice] = (ADC_ChannelGetData(0) >> 4) & 0xFFF;
        indice = (indice + 1)%2048; //8192 bytes en palabras de 4 bytes
    }
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
    mat01.matchValue = 5;

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
}

void initTimer1(void) {
    TIM_TIMERCFG_T tim1;
    tim1.PrescaleOption = TIM_PRESCALE_USVAL;
    tim1.PrescaleValue = 1000; // 1ms

    TIM_MATCHCFG_T mat10;
    mat10.MatchChannel = 0;
    mat10.IntOnMatch = ENABLE;
    mat10.ResetOnMatch = ENABLE;
    mat10.MatchValue = 1000; // 1000ms = 1 segundo

    TIM_InitTimer(LPC_TIM1, &tim1);
    TIM_ConfigMatch(LPC_TIM1, &mat10);
    TIM_Cmd(LPC_TIM1, ENABLE);
    NVIC_EnableIRQ(TIMER1_IRQn);
}

void TIMER1_IRQHandler(void) {
    if (TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT)) {
        if (modoDMA == 1) {
            // Caso DMA: Iniciamos el canal 0 (ADC a Memoria)
            // El DMA transferirá 2048 muestras y se detendrá (o seguirá si hay LLI)
            GPDMA_ChannelCmd(0, ENABLE); 
        } else {
            // Caso Punteros: Habilitamos la interrupción del ADC
            // La recolección ocurrirá en la ADC_IRQHandler hasta llenar el buffer
            indice = 0; 
            ADC_IntEnable(LPC_ADC, ADC_INT_CH0, ENABLE);
        }
        TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    }
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) &(LPC_ADC -> ADDR0);
    lli.dstAddr = (uint32_t) inicio;
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 2048 | (2 << 18) | (2 << 21) | (1 << 27); 

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 2048;
    dma.type = GPDMA_P2M;
    dma.scrMemAddr = 0;
    dma.dstMemAddr = (uint32_t) inicio;
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
    //no lo iniciamos aun
}

void EINT2_IRQHandler(void)
{
    if(numInt == 0)
    {
        numInt = 1;
        ADC_IntDisable(ADC_INT_CH0);
        GPDMA_ChannelStart(0);
    }

    EXTI_ClearFlag(EXTI_EINT2);
}

void initGPDMA_M2M(void)
{
    GPDMA_LLI_T lli1;
    lli1.scrAddr = (uint32_t) inicio;
    lli1.dstAddr = (uint32_t) inicioM2M;
    lli1.nextLLI = (uint32_t) &lli1;
    lli1.control = 2048 | (2 << 18) | (2 << 21) | (1 << 27); 

    GPDMA_Channel_CFG_T dma1;
    dma1.channelNum = 0;
    dma1.transferSize = 2048;
    dma1.type = GPDMA_P2M;
    dma1.scrMemAddr = (uint32_t) inicio;
    dma1.dstMemAddr = (uint32_t) inicioM2M;
    dma1.scrConn = GPDMA_ADC;
    dma1.dstConn = 0;
    dma1.scr.width = GPDMA_WORD;
    dma1.scr.burst = GPDMA_BSIZE_1;
    dma1.scr.increment = DISABLE;
    dma1.dst.width = GPDMA_WORD;
    dma1.dst.burst = GPDMA_BSIZE_1;
    dma1.dst.increment = ENABLE;
    dma1.intTC = ENABLE;
    dma1.intErr = DISABLE;
    dma1.linkedList = (uint32_t) &lli1;

    GPDMA_Init();
    GPDMA_SetupChannel(&dma1);
    GPDMA_ChannelStart(1);
    NVIC_EnableIRQn(DMA_IRQn);
}

void DMA_IRQHandler(void)
{
    if(GPDMA_IntGetStatus(GPDMA_INTTC, 1))
    {
        uint32_t sumatoria = 0;

        for(int i = 0; i < 2048; i++)
        {
            sumatoria = sumatoria + (inicioM2M[i] >> 4)&0xFFF;
        }

        promedio = sumatoria/2048;
    }

    GPDMA_ClearIntPending(GPDMA_CLR_INTTC, 1);
}

//rehacer