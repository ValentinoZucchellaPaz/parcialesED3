#include "LPC17xx.h"

#define MOTOR (1 << 0)   // P0.0
#define ENC_PIN (1 << 2) // P1.26 como CAP0.0 (ejemplo)

//-------------------- ESTADO --------------------//
typedef enum {
    MOVIENDO,
    ESPERANDO
} estado_t;

volatile estado_t estado = MOVIENDO;

//-------------------- GPIO --------------------//
void iniciarGPIO(void)
{
    // Motor como salida
    LPC_GPIO0->FIODIR |= MOTOR;
    LPC_GPIO0->FIOCLR = MOTOR; // apagado inicial

    // Configurar P1.26 como CAP0.0 (entrada contador)
    LPC_PINCON->PINSEL3 &= ~(3 << 20);
    LPC_PINCON->PINSEL3 |=  (3 << 20); // función CAP0.0
}

//-------------------- TIMER0 (CONTADOR) --------------------//
void iniciarContador(void)
{
    LPC_SC->PCONP |= (1 << 1); // Timer0

    // Modo contador por flanco de subida en CAP0.0
    LPC_TIM0->CTCR = 1;

    LPC_TIM0->PR = 0;
    
    LPC_TIM0->MR0 = 250; // 250 pulsos = 50 cm
    LPC_TIM0->MCR = (1 << 0) | (1 << 1); // interrupción + reset

    LPC_TIM0->TCR = 2;
    LPC_TIM0->TCR = 1;

    NVIC_EnableIRQ(TIMER0_IRQn);
}

//-------------------- TIMER1 (2 SEGUNDOS) --------------------//
void iniciarTimer1(void)
{
    LPC_SC->PCONP |= (1 << 2); // Timer1

    LPC_SC->PCLKSEL0 &= ~(3 << 4); // PCLK/4 = 25 MHz

    LPC_TIM1->PR = 24999; // 1 ms

    LPC_TIM1->MR0 = 2000; // 2000 ms = 2 s

    LPC_TIM1->MCR = (1 << 0) | (1 << 1); // interrupción + reset
}

//-------------------- MOTOR --------------------//
void motorON(void)
{
    LPC_GPIO0->FIOSET = MOTOR;
}

void motorOFF(void)
{
    LPC_GPIO0->FIOCLR = MOTOR;
}

//-------------------- ISR TIMER0 --------------------//
void TIMER0_IRQHandler(void)
{
    if(LPC_TIM0->IR & (1 << 0))
    {
        LPC_TIM0->IR = (1 << 0);

        motorOFF();              // llegó a 50 cm
        estado = ESPERANDO;

        // iniciar Timer1 desde 0
        LPC_TIM1->TCR = 2;
        LPC_TIM1->TCR = 1;
    }
}

//-------------------- ISR TIMER1 --------------------//
void TIMER1_IRQHandler(void)
{
    if(LPC_TIM1->IR & (1 << 0))
    {
        LPC_TIM1->IR = (1 << 0);

        motorON();               // volver a mover cinta
        estado = MOVIENDO;

        // reiniciar contador
        LPC_TIM0->TCR = 2;
        LPC_TIM0->TCR = 1;
    }
}

//-------------------- MAIN --------------------//
int main(void)
{
    iniciarGPIO();
    iniciarContador();
    iniciarTimer1();

    NVIC_EnableIRQ(TIMER1_IRQn);

    motorON(); // arranca moviendo

    while(1)
    {
        // todo se maneja por interrupciones
    }
}