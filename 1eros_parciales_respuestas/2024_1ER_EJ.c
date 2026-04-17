#include "LPC17xx.h"

typedef enum
{
    esperarAuto,
    validarTicket,
    luzRoja,
    boton,
    motor
} Estado;

volatile uint32_t t = 0;
volatile uint32_t t_ref = 0;
uint32_t t_motor = 0;
uint8_t ticketValido = 1;
uint8_t boton_counter = 0;
volatile Estado estado = esperarAuto;

uint8_t boton_anterior = 0;
uint32_t t_antiRebote = 0;

void iniciarGPIO(void)
{
    //P2.04 COMO INTERRUPCION
    LPC_PINCON -> PINSEL4 &= ~(3 << 8);
    LPC_GPIO2 -> FIODIR &= ~(1 << 4);

    //P0.15 COMO SALIDA MOTOR
    LPC_PINCON -> PINSEL0 &= ~(3 << 30);
    LPC_GPIO0 -> FIODIR |= (1 << 15);
    LPC_PINCON -> PINMODE0 &= ~(3 << 30);
    LPC_PINCON -> PINMODE0 |= (3 << 30); //PULLDOWN

    //P1.05 COMO SALIDA LED
    LPC_PINCON -> PINSEL2 &= ~(3 << 10);
    LPC_GPIO1 -> FIODIR |= (1 << 5);
    LPC_PINCON -> PINMODE2 &= ~(3 << 10);
    LPC_PINCON -> PINMODE2 |= (3 << 10); //PULLDOWN

    //P3.04 COMO ENTRADA BOTON
    LPC_PINCON -> PINSEL6 &= ~(3 << 8);
    LPC_GPIO3 -> FIODIR &= ~(1 << 4);
    LPC_PINCON -> PINMODE6 &= ~(3 << 8);
    LPC_PINCON -> PINMODE6 |= (3 << 8); //PULLDOWN
}

void iniciarSysTick(void)
{
    SysTick -> LOAD = ((70000000/1000 * 10) - 1); //10ms
    SysTick -> VAL = 0;
    SysTick -> CTRL |= 7;
}

void iniciarEINT3(void)
{
    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_SetPriority(EINT3_IRQn, 1);
    
    LPC_GPIOINT -> IO2IntEnF |= (1 << 4); //suponemos pullup por hardware
    LPC_GPIOINT -> IO2IntClr = (1 << 4);
}

void SysTick_Handler(void)
{
    t++;
}

void EINT3_IRQHandler(void)
{
    if(estado == esperarAuto)
    {
        estado = validarTicket;
    }

    LPC_GPIOINT -> IO2IntClr = (1 << 4);
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
            case esperarAuto:
            t = 0;
            t_ref = 0;
            boton_counter = 0;
            t_motor = 0;

            boton_anterior = 0;
            t_antiRebote = 0;

            LPC_GPIO1 -> FIOCLR = (1 << 5);
            LPC_GPIO0 -> FIOCLR = (1 << 15);
            break;

            case validarTicket:
            if(ticketValido == 0)
            {
                estado = luzRoja;
                t_ref = t;
                LPC_GPIO1 -> FIOSET = (1 << 5);
            }
            else
            {
                t_ref = t;
                estado = boton;
            }
            break;

            case luzRoja:
            if(t - t_ref >= 1000) //10seg de luz roja (esto no es de la consigna, solo lo crei necesario)
            {
                LPC_GPIO1 -> FIOCLR = (1 << 5);
                estado = esperarAuto;
            }
            break;

            case boton:
            if(t - t_ref <= 300)
            {
                uint8_t estado_boton = (LPC_GPIO3 -> FIOPIN & (1 << 4)) ? 0:1;

                if(estado_boton == 1 && boton_anterior == 0) //detecto flanco positivo
                {
                    if(t - t_antiRebote >= 2) //20ms de antirebote
                    {
                        t_antiRebote = t;
                        boton_counter++;
                    }
                }

                boton_anterior = estado_boton;
            }
            else
            {
                switch(boton_counter)
                {
                    case 0:
                    t_motor = 500;
                    break;

                    case 1:
                    t_motor = 1000;
                    break;

                    case 2:
                    t_motor = 2000;
                    break;

                    case 3:
                    t_motor = 4000;
                    break;

                    case 4:
                    t_motor = 500;
                    break;

                    default:
                    t_motor = 500;
                    break;
                }

                LPC_GPIO0 -> FIOSET = (1 << 15);
                t_ref = t;
                estado = motor;
            }

            break;

            case motor:
            if(t - t_ref >= t_motor)
            {
                LPC_GPIO0 -> FIOCLR = (1 << 15);
                estado = esperarAuto;
            }
            break;
        }
    }
}