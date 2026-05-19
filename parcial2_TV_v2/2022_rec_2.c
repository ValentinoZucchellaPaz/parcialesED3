#include "LPC17xx.h"
#include "LPC17xx_adc.h"

void initADC(void)
{
    ADC_PowerUp();
    ADC_Init(200000); //frecuencia de trabajo, no de muestreo
    ADC_PinConfig(0);
    ADC_ChannelEnable(0);
    ADC_StartCmd(ADC_START_ON_MAT01);
    ADC_EdgeStartConfig(ADC_START_ON_RISING);
    ADC_IntEnable(ADC_INT_CH0);
    NVIC_EnableIRQ(ADC_IRQn);
}

/*
    ADC0Value = ADC_ChannelGetData(0, ADC_DATA_DONE);
*/

    uint8_t N = 10;
    uint32_t muestrasDiez[20];
    uint8_t indice = 0;
    uint8_t numMuestras = 0;

    if(N == 10)
    {
        numMuestras++;

        if(numMuestras >= 10)
        {
            numMuestras = 0;
            muestrasDiez[indice] = filtSignal;
            indice ++;
            if(indice >= 20) {indice = 0;}
        }
    }

    //esto se repite en bucle

/*
    si mantenemos la frec. del adc fija y agregamos mas canales de conversion, esta misma frecuencia se va a tener que dividir
    entre todos los canales. Afectando la frecuencia de muestreo
*/
    
    