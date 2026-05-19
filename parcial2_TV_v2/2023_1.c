#include "LPC17xx.h"
#include "LPC17xx_timer.h"
#include "LPC17xx_pinsel.h"
#include "LPC17xx_gpdma.h"
#include "LPC17xx_dac.h"

volatile uint8_t nivel = 0;
uint32_t periodo = 1; // tiempo_alto - ultimo_alto
uint32_t tiempo_alto = 0;
uint32_t tiempo_ENalto = 0; //tiempo_bajo - ultimo_alto
uint32_t tiempo_bajo = 0; 
uint32_t ultimo_alto = 0;
uint32_t registros[10] = {0};
uint8_t indice = 0;
uint32_t periodo = 0;
uint32_t valorDAC = 0;
uint32_t promedio = 0;

void initPin(void)
{
    PINSEL_CFG_T pinCap;
    pinCap.port = 1;
    pinCap.pin = 26;
    pinCap.func = PINSEL_FUNC_10;
    pinCap.mode = PINSEL_PULLDOWN;
    pinCap.openDrain = DISABLE;
    PINSEL_ConfigPin(&pinCap);
}

void initTimer0(void)
{
    TIM_TIMERCFG_T tim0;
    tim0.prescaleOpt = TIM_US;
    tim0.prescaleValue = 1; 

    TIM_CAPTURECFG_T cap00;
    cap00.channel = 0;
    cap00.risingEn = ENABLE;
    cap00.fallingEn = ENABLE;
    cap00.intEn = ENABLE;

    NVIC_EnableIRQ(TIMER0_IRQn);
    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigCapture(LPC_TIM0, &cap00);
    TIM_Enable(LPC_TIM0);
}

void TIMER0_IRQHandler(void)
{
    if(TIM_GetIntStatus(LPC_TIM0, TIM_CR0_INT))
    {
        if(nivel == 0)
        {
            tiempo_alto = TIM_GetCaptureValue(LPC_TIM0, 0);
            periodo = tiempo_alto - ultimo_alto;
            ultimo_alto = tiempo_alto;
            nivel = 1;

            registros[indice] = (tiempo_ENalto*100)/periodo;

            indice = (indice + 1)%10;

            uint32_t sumatoria = 0;

            for(int i = 0; i < 10; i++)
            {
                sumatoria = registros[i] + sumatoria;
            }

            promedio = sumatoria/10;

            valorDAC = promedio * 620/100;
        }
        else if(nivel == 1)
        {
            tiempo_bajo = TIM_GetCaptureValue(LPC_TIM0, 0);
            tiempo_ENalto = tiempo_bajo - ultimo_alto;
            nivel = 0;
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
    DAC_ConfigDAConverterControl(&dac);
    DAC_SetDMATimeOut(12500000); //suponiendo pclk de 25mhz;
}

void initGPDMA(void)
{
    GPDMA_LLI_T lli;
    lli.scrAddr = (uint32_t) &valorDAC;
    lli.dstAddr = (uint32_t) &(LPC_DAC -> DACR);
    lli.nextLLI = (uint32_t) &lli;
    lli.control = 1 | (2 << 18) | (2 << 21); //paso un solo dato (ValorDAC)

    GPDMA_Channel_CFG_T dma;
    dma.channelNum = 0;
    dma.transferSize = 1;
    dma.type = GPDMA_M2P;
    dma.scrMemAddr = (uint32_t) &valorDAC;
    dma.dstMemAddr = 0;
    dma.scrConn = 0;
    dma.dstConn = GPDMA_DAC;
    dma.scr.width = GPMDA_WORD;
    dma.scr.burst = GPDMA_BSIZE_1;
    dma.scr.increment = DISABLE;
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





//alternativa
#include "LPC17xx.h"
// ... otras librerías ...

volatile uint32_t valorDAC = 0; // Global para que el DMA la lea
uint32_t capturas[10] = {0};
uint8_t idx = 0;

void EINT3_IRQHandler(void) { // Las interrupciones de GPIO caen aquí
    static uint32_t t_subida = 0;
    uint32_t t_actual = LPC_TIM0->TC; // Usamos el Timer solo como contador de tiempo

    // Si el pin P0.26 está en alto, es un flanco de subida
    if (GPIO_ReadValue(0) & (1 << 26)) {
        uint32_t periodo = t_actual - t_subida;
        // Solo calculamos si el periodo es válido (evitar primer ciclo)
        if (periodo > 0) {
            // No calculamos el Duty aquí porque t_bajada no se ha actualizado
        }
        t_subida = t_actual;
    } 
    else { // Flanco de bajada
        uint32_t t_on = t_actual - t_subida;
        // Suponiendo que el periodo es constante o lo medimos:
        // Para este ejercicio simplificado, calculemos el duty cada ciclo completo:
        // Pero el ejercicio pide el promedio de las últimas 10 capturas.
    }
    
    LPC_GPIOINT->IO0IntClr = (1 << 26);
}


//REHACER