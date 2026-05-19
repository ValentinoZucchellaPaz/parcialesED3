#include "LPC17xx.h"
#include "LPC17xx_adc"
#include "LPC17xx_timer.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_pinsel.h"

volatile uint32_t valoresADC[4] = {0};
uint32_t sumatoria = 0;
uint32_t promedio = 0;
uint32_t ultimo_promedio = 9999;

void initADC(void)
{
    ADC_Init(200000);
    ADC_PinCongif(0);
    ADC_PowerUp();
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_ChannelEnable(0);
    ADC_EdgeStartConfig(ADC_START_ON_RISING);
}

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 1000; //1ms

    TIM_MATCHCFG_T mat01;
    mat01.channel = 1;
    mat01.intEn = DISABLE;
    mat01.stopEn = DISABLE;
    mat01.resetEn = ENABLE;
    mat01.extOpt = TIM_TOGGLE;
    mat01.matchValue = 30000; //30seg

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}

void initTimer1(void)
{
    TIM_TIMERCFG_T tim1;
    tim1.prescaleOpt = TIM_US;
    tim1.prescaleValue = 10000; //10ms

    TIM_MATCHFG_T mat10;
    mat10.channel = 0;
    mat10.intEn = ENABLE;
    mat10.stopEn = DISABLE;
    mat10.resetEn = ENABLE;
    mat10.extOpt = TIM_NOTHING;
    mat10.matchValue = 12000; //2min

    TIM_InitTimer(LPC_TIM1, &tim1);
    TIM_ConfigMatch(LPC_TIM1, &mat10);
    NVIC_EnableIRQ(TIM1_IRQn);
    TIM_Enable(LPC_TIM1);
}

void initGPMDA(void)
{
    GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) &(LPC_ADC -> ADDR0);
    lli.dstAddr = (uint32_t) &(valoresADC);
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 4 | (2 << 18) | (2 << 21) | (1 << 27); //tamaño 4, ancho 32 palabras, increment dst, burst 1 (valor por defecto?)

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 4;
    dma.type = GPDMA_P2M;
    dma.scrMemAddr = 0;
    dma.dstMemAddr = (uint32_t) &(valoresADC);
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
    dma.linkedList = (uint32_t) &(lli);

    GPDMA_Init();
    GPDMA_SetupChannel(&dma);
    GPDMA_ChannelStart(0);
}

void TIMER1_IRQHandler(void)
{
    if(TIM_GetIntStatus(LPC_TIM1, TIM_MR0_INT))
    {
        sumatoria = 0;
        for(int i = 0; i < 4; i++)
        {
            sumatoria = sumatoria + (valoresADC[i] >> 4)&0xFFF;
        }

        promedio = sumatoria/4;

        TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
    }
}

void initTimer3_PWM_Interrupt(void) {
    TIM_TIMERCFG_T tim3;
    tim3.PrescaleOpt = TIM_PRESCALE_USVAL;
    tim3.PrescaleValue = 1; // Resolución de 1us

    // MATCH 0: PERIODO (50us para 20kHz)
    TIM_MATCHCFG_T mat30;
    mat30.MatchChannel = 0;
    mat30.IntOnMatch = ENABLE;    // ¡HABILITADO! Interrumpe al final del ciclo
    mat30.StopOnMatch = DISABLE;
    mat30.ResetOnMatch = ENABLE;  // Resetea el contador automáticamente a 0
    mat30.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    mat30.MatchValue = 50; 

    // MATCH 1: DUTY CYCLE (Ancho de pulso dinámico)
    TIM_MATCHCFG_T mat31;
    mat31.MatchChannel = 1;
    mat31.IntOnMatch = ENABLE;    // ¡HABILITADO! Interrumpe a mitad de camino
    mat31.StopOnMatch = DISABLE;
    mat31.ResetOnMatch = DISABLE; // No resetea, el reset lo maneja el Match 0
    mat31.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
    mat31.MatchValue = 25;        // Valor inicial al 50%

    TIM_InitTimer(LPC_TIM3, TIM_TIMER_MODE, &tim3);
    TIM_ConfigMatch(LPC_TIM3, &mat30);
    TIM_ConfigMatch(LPC_TIM3, &mat31);
    
    NVIC_EnableIRQ(TIMER3_IRQn);  // Habilitamos las interrupciones del Timer 3
    TIM_Cmd(LPC_TIM3, ENABLE);
}

// HANDLER DEL TIMER 3: Aquí ocurre la magia del PWM por Software
void TIMER3_IRQHandler(void) {
    // 1. ¿La interrupción fue por el Match 0 (Periodo)?
    if (TIM_GetIntStatus(LPC_TIM3, TIM_MR0_INT)) {
        LPC_GPIO0->FIOSET = (1 << 11); // El match de periodo te pone el pin en 1
        TIM_ClearIntPending(LPC_TIM3, TIM_MR0_INT);
    }
    
    // 2. ¿La interrupción fue por el Match 1 (Duty Cycle)?
    if (TIM_GetIntStatus(LPC_TIM3, TIM_MR1_INT)) {
        LPC_GPIO0->FIOCLR = (1 << 11); // El match de duty cycle te pone el pin en 0
        TIM_ClearIntPending(LPC_TIM3, TIM_MR1_INT);
    }
}

// Función para cambiar el Duty Cycle modificando el Match 1 al vuelo
void updatePWM_Duty(uint32_t t) {
    LPC_TIM3->MR1 = t; 
}

void initPin(void)
{
    PINSEL_CFG_T pinTim3;
    pinTim3.port = 0;
    pinTim3.pin = 11;
    pinTim3.func = PINSEL_FUNC_00;
    pinTim3.mode = PINSEL_PULLUP;
    pinTim3.openDrain = DISABLE;
    PINSEL_ConfigPin(&pinTim3);
    GPIO_SetDir(0, (1 << 11), GPIO_OUTPUT);
    GPIO_ClearPins(0, (1 << 11));
}

int main(void)
{
    initADC();
    initGPMDA();
    initTimer0();
    initTimer1();
    initPin();
    initTimer3_PWM_Interrupt();

    while(1) {
        if (promedio != ultimo_promedio) {
            ultimo_promedio = promedio;

            if (promedio < 310) { // Menor a 1V (Forzar 0% absoluto)
                NVIC_DisableIRQ(TIMER3_IRQn); // Apagamos interrupciones para congelar el estado
                LPC_GPIO0->FIOCLR = (1 << 11); // Pin apagado fijo
            } 
            else if (promedio >= 310 && promedio <= 620) { // Rango intermedio de PWM
                // Mapeo lineal: promedio (310 a 620) -> t (25us a 45us)
                uint32_t valorPWM = 25 + ((promedio - 310) * (45 - 25)) / (620 - 310);
                
                updatePWM_Duty(valorPWM);
                NVIC_EnableIRQ(TIMER3_IRQn);  // Asegura que las interrupciones estén activas
            } 
            else { // Mayor a 2V (Forzar 100% absoluto)
                NVIC_DisableIRQ(TIMER3_IRQn); // Apagamos interrupciones
                LPC_GPIO0->FIOSET = (1 << 11); // Pin encendido fijo
            }
        }
    }
}


//REHACER


