/*
 * Secuencia temporizada con Timer0 (Match + interrupciones)
 * 
 * Descripción:
 * - Controla dos LEDs:
 *      LED1 en P0.22 → Toggle cada 200 ms
 *      LED2 en P0.21 → Toggle cada 1 segundo
 * 
 * - Utiliza únicamente el Timer0
 * - Genera interrupciones por Match (MR0 y/o MR1)
 * - No utiliza retardos por software (delay, for, while)
 * 
 * Consideraciones:
 * - Configurar PCONP para habilitar Timer0
 * - Configurar PCLKSEL0 para definir la frecuencia del timer
 * - Usar MR0 (y opcionalmente MR1) para generar eventos temporizados
 * - Configurar MCR para interrupciones (y reset según estrategia)
 * - Limpiar flags en IR dentro de la ISR
 * - Habilitar interrupción en NVIC
 * 
 * Objetivo:
 * - Comprender el uso de múltiples Match Registers
 * - Diseñar temporizaciones distintas con un solo timer
 * - Analizar el efecto del reset del timer sobre distintos MR
 */

#include "LPC17xx.h"


void iniciarTIMER0(void)
{
    LPC_SC -> PCONP |= (1 << 1);
    LPC_SC -> PCLKSEL0 &= ~(3 << 2); //PCLK/4
    LPC_TIM0 -> PR = 0;

    LPC_PINCON -> PINSEL3 &= ~(3 << 24);
    LPC_PINCON -> PINSEL3 |= (3 << 24); //P1.28 COMO MATCH TIM0

    LPC_TIM0 -> MR0 = 25000000; //1SEG

    LPC_TIM0 -> EMR &= ~(3 << 4);
    LPC_TIM0 -> EMR |= (3 << 4); //P1.28 COMO TOGGLE MAT0.0, EL LED ESTA CONECTADO ACA

    LPC_TIM0 -> MCR |= (1 << 1); //SE RESETEA EL TC CUANDO HAY MATCH

    LPC_TIM0 -> TCR = (1 << 1); //RESET TIM0
    LPC_TIM0 -> TCR = (1 << 0); //ENABLE TIM0 (ORDEN RECOMENDADO) 
}
