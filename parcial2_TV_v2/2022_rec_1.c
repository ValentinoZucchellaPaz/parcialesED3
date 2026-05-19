#include "LPC17xx.h"
#include "LPC17xx_timer.h"

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
    mat01.extOpt = TIM_TOGGLE; //toggle para activar ADC, adc debe estar configurado para usar el mat00
    mat01.matchValue = 10; //10ms, necesario para 100hz

    TIM_InitTimer(LPC_TIM0, &tim0);
    TIM_ConfigMatch(LPC_TIM0, &mat01);
    TIM_Enable(LPC_TIM0);
}


/*
en el handler se guarda el valor del adc en el adc0value, se rota el adcvolt para liberar un espacio, se guarda
el valor de adc0value estandarizado en adcvolt[0] y se guarda el resultado final en filtsignal, que es una
convolucion de la señal con la respuesta impulsiva del filtro
*/