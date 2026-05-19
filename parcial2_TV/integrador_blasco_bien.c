/*
 a ver, te voy a describir el codigo completo y vos hacelo inspirado en lo que te dije

crea el pin y la exti como yo

crea el adc pero sin interrupcion

creal el timer0 como yo

crea un canal 0 de dma que lleve los datos del adc a la primera mitad del banco1

crea el exti0handler igual que yo

crea un timer 1 que haga interrupcion cada 1 seg y que en esta interrupcion, segun el estado, se haga un ciclo for con 
punteros para pasar los datos o se habilite un canal 1 de dma que haga lo mismo (tenes que crear este canal)

una vez se pasen los datos (verifica que el ciclo for haya acabado o por interrupcion de dma) hay que promediarlos 
y guardarlos en la 2da mitad del banco 0 usando una funcion promediarADC() que, ademas de sacar el promedio, 
lo guarda con punteros en la segunda mitad del banco 0 usando un indice rotativo 
*/

#include "LPC17xx.h"
#include "LPC17xx_adc.h"
#include "LPC17xx_timer.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_exti.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_uart.h"

// Direcciones de memoria (LPC1769)
uint32_t* const ADC_CAPTURE_RAM = (uint32_t*) 0x20080000; // Banco 1 (SRAM1)
uint32_t* const PROCESAMIENTO_RAM = (uint32_t*) 0x2007C000; // Banco 0 (SRAM0) Inicio
uint32_t* const HISTORIAL_PROMEDIOS = (uint32_t*) 0x2007E000; // Banco 0 (SRAM0) Mitad

volatile uint8_t estado = 0; // 0: Punteros, 1: DMA
volatile uint32_t indiceP = 0;
volatile uint8_t dma_transfer_done = 0;

// Prototipos
void promediarADC(void);

void initPin(void) {
    PINSEL_CFG_T pin;
    pin.port = 2; pin.pin = 10;
    pin.func = 1; // EINT0
    pin.mode = PINSEL_PULLDOWN;
    PINSEL_ConfigPin(&pin);

    EXTI_InitTypeDef eint0;
    eint0.EXTI_Line = EXTI_EINT0;
    eint0.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
    eint0.EXTI_trigger = EXTI_TRIGGER_RISING;
    
    EXTI_Init();
    EXTI_Config(&eint0);
    NVIC_EnableIRQ(EINT0_IRQn);
}

void initADC(void) {
    ADC_Init(200000);
    ADC_PinConfig(0); // P0.23
    ADC_IntEnable(ADC_INT_CH0, DISABLE); // Sin interrupción de ADC
    ADC_ChannelEnable(0);
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_EdgeStartConfig(ADC_START_ON_RISING);
}

void initTimer0(void) {
    TIM_TIMERCFG_T tim;
    tim.prescaleOpt = TIM_PRESCALE_USVAL;
    tim.prescaleValue = 1;

    TIM_MATCHCFG_T mat;
    mat.matchValue = 3; // 3us para Nyquist
    mat.extOpt = TIM_TOGGLE;
    mat.resetEn = ENABLE;
    
    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &tim);
    TIM_ConfigMatch(LPC_TIM0, &mat);
    TIM_Enable(LPC_TIM0);
}

void initGPDMA_Canal0(void) {
    static GPDMA_LLI_T lli0;
    lli0.SrcAddr = (uint32_t) &(LPC_ADC->ADDR0);
    lli0.DstAddr = (uint32_t) ADC_CAPTURE_RAM;
    lli0.NextLLI = (uint32_t) &lli0;
    lli0.Control = 2048 | (2 << 18) | (2 << 21) | (1 << 27); // Word, Dest Inc

    GPDMA_Channel_CFG_T cfg;
    cfg.ChannelNum = 0;
    cfg.TransferSize = 2048;
    cfg.TransferType = GPDMA_TRANSFERTYPE_P2M;
    cfg.SrcConn = GPDMA_CONN_ADC;
    cfg.DstMemAddr = (uint32_t) ADC_CAPTURE_RAM;
    cfg.DMALLI = (uint32_t) &lli0;

    GPDMA_Init();
    GPDMA_Setup(&cfg);
    GPDMA_ChannelCmd(0, ENABLE);
}

void initTimer1(void) {
    TIM_TIMERCFG_T tim;
    tim.prescaleOpt = TIM_PRESCALE_USVAL;
    tim.prescaleValue = 1;

    TIM_MATCHCFG_T mat;
    mat.matchValue = 1000000; // 1 segundo
    mat.intEn = ENABLE;
    mat.resetEn = ENABLE;

    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &tim);
    TIM_ConfigMatch(LPC_TIM1, &mat);
    TIM_Enable(LPC_TIM1);
    NVIC_EnableIRQ(TIMER1_IRQn);
}

void EINT0_IRQHandler(void) {
    if (EXTI_GetEXTIFlag(EXTI_EINT0)) {
        estado = (estado == 0) ? 1 : 0;
        EXTI_ClearEXTIFlag(EXTI_EINT0);
    }
}

void GPDMA_IRQHandler(void) {
    if (GPDMA_IntGetStatus(GPDMA_STAT_INTTC, 1)) {
        dma_transfer_done = 1;
        GPDMA_ClearIntPending(GPDMA_CLR_INTTC, 1);
    }
}

void TIMER1_IRQHandler(void) {
    if (TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT)) {
        if (estado == 0) {
            // MODO PUNTEROS
            for (int i = 0; i < 2048; i++) {
                PROCESAMIENTO_RAM[i] = ADC_CAPTURE_RAM[i];
            }
            promediarADC();
        } 
        else {
            // MODO DMA (Canal 1 - M2M)
            GPDMA_Channel_CFG_T cfgM2M;
            cfgM2M.ChannelNum = 1;
            cfgM2M.TransferSize = 2048;
            cfgM2M.TransferType = GPDMA_TRANSFERTYPE_M2M;
            cfgM2M.SrcMemAddr = (uint32_t) ADC_CAPTURE_RAM;
            cfgM2M.DstMemAddr = (uint32_t) PROCESAMIENTO_RAM;
            cfgM2M.DMALLI = 0;

            dma_transfer_done = 0;
            GPDMA_Setup(&cfgM2M);
            NVIC_EnableIRQ(DMA_IRQn);
            GPDMA_ChannelCmd(1, ENABLE);
            // El promedio se llamará cuando dma_transfer_done sea 1 (ver main)
        }
        TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    }
}

void promediarADC(void) {
    uint64_t sumatoria = 0;
    for (int i = 0; i < 2048; i++) {
        sumatoria += (PROCESAMIENTO_RAM[i] >> 4) & 0xFFF;
    }
    
    HISTORIAL_PROMEDIOS[indiceP] = (uint32_t)(sumatoria / 2048);
    indiceP = (indiceP + 1) % 2048; // Indice rotativo
}

int main(void) {
    initPin();
    initADC();
    initTimer0();
    initGPDMA_Canal0();
    initTimer1();

    while (1) {
        if (estado == 1 && dma_transfer_done == 1) {
            dma_transfer_done = 0;
            promediarADC();
        }
    }
}