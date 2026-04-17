#include "LPC17xx.h"

typedef enum
{
    puertaCerrada,
    esperandoCodigo,
    alarma
} Estado;

volatile Estado estado = puertaCerrada;
volatile uint32_t t = 0;
uint32_t t_ref = 0;
uint8_t intentos = 0;
uint8_t inputAnterior = 0;

void iniciarGPIO(void)
{
    //PO.06 COMO ENTRADA INT SENSOR
    LPC_PINCON -> PINSEL0 &= ~(3 << 12);
    LPC_GPIO0 -> FIODIR &= ~(1 << 6);

    //P2.00 - P2.03 COMO ENTRADAS DIP SWITCH
    LPC_PINCON -> PINSEL4 &= ~0xFF;
    LPC_GPIO2 -> FIODIR &= ~0xF;

    //P1.11 COMO SALIDA BUZZER
    LPC_PINCON -> PINSEL2 &= ~(3 << 22);
    LPC_PINCON -> PINMODE2 &= ~(3 << 22);
    LPC_PINCON -> PINMODE2 |= (3 << 22); //PULL DOWN
    LPC_GPIO1 -> FIODIR |= (1 << 11);
    LPC_GPIO1 -> FIOCLR = (1 << 11);
}

void iniciarSysTick(void)
{
    SysTick -> LOAD = ((100000000/1000 * 10) - 1); //10ms
    SysTick -> VAL = 0;
    SysTick -> CTRL |= 7;
}

void iniciarEINT3(void)
{
    NVIC_Enable(EINT3_IRQn);
    NVIC_SetPriority(EINT3_IRQn, 1);

    LPC_GPIOINT -> IO0IntEnF |= (1 << 6); //suponemos pullup externo
    LPC_GPIOINT -> IO0IntClr = (1 << 6);
}

void SysTick_Handler(void)
{
    t++;
}

void EINT3_IRQHandler(void)
{
    if(estado == puertaCerrada)
    {
        t_ref = t;
        estado = esperandoCodigo;
    }

    LPC_GPIOINT -> IO0IntClr = (1 << 6);
}

int main(void)
{
    iniciarGPIO();
    iniciarSysTick();
    iniciarEINT3();

    while(1)
    {
        switch(estado)
        {
            case puertaCerrada:
            t = 0;
            t_ref = 0;
            intentos = 0;
            inputAnterior = 0;
            break;

            case esperandoCodigo:
            if(t - t_ref > 3000 || intentos >= 2)
            {
                estado = alarma;
            }

            uint8_t input = LPC_GPIO2 -> FIOPIN & 0xF;

            if(input != inputAnterior)
            {
                if(input == 0xA)
                {
                    estado = puertaCerrada;
                }
                else
                {
                    intentos++;
                }
            }

            inputAnterior = input;

            break;

            case alarma:
            LPC_GPIO1 -> FIOSET = (1 << 11); //se debe reiniciar el lpc para apagar la alarma
            break;

        }
    }
}