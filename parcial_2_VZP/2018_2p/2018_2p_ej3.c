/**
Considerando que se tiene un bloque de datos comprendidos entre las posiciones de memorias, dirección inicial= 0x10000800 a la dirección final= 0x10001000 ambas inclusive y se desea trasladar este bloque de datos una nueva zona de memoria comprendida entre la dirección inicial= 0x10002800 y la dirección Final=0x10003000 (en el mismo orden). Teniendo en cuenta además que los datos contenidos dentro de la zona de memoria son de 16 bits (AHB Master endianness configuration - por defecto) y que estos deben moverse de a uno (1) en cada evento de DMA, se sincronizará la transmisión con evento de match0 del timer1.

Se pide que Enumere y explique los puntos a tener en cuenta para configurar correctamente el controlador DMA.
*/

#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpdma.h"

void configDMA(void)
{
    GPDMA_Channel_CFG_T chCfg = {
        .channelNum = 0,
        .transferSize = 256,
        .type = GPDMA_M2P,
        .src = {.burst = GPDMA_BSIZE_1, .increment = ENABLE, .width = GPDMA_HALFWORD},
        .dst = {.burst = GPDMA_BSIZE_1, .increment = ENABLE, .width = GPDMA_HALFWORD},
        .srcMemAddr = (uint32_t)0x10000800,
        .dstConn = TIM_MAT0_0_P3_25};
    GPDMA_Init();
    GPDMA_SetupChannel(&chCfg);
    GPDMA_ChannelStart(GPDMA_CH_0);
    LPC_GPDMACH0->DMACCDestAddr = 0x10002800;
}

void configTimer(void)
{
    TIM_TIMERCFG_T timCfg = {
        .prescaleOpt = TIM_US,
        .prescaleValue = 1};
    TIM_MATCHCFG_T matchCfg = {
        .channel = TIM_MATCH_0,
        .extOpt = TIM_TOGGLE,
        .matchValue = 10,
        .resetEn = ENABLE};

    TIM_InitTimer(LPC_TIM0, &timCfg);
    TIM_ConfigMatch(LPC_TIM0, &matchCfg);
    TIM_Enable(LPC_TIM0);
}

int main(void)
{
    configDMA();
    configTimer();

    while (1)
    {
    }
    return 0;
}