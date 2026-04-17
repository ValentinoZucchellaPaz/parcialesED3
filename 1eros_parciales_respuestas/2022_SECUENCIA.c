/*
===============================================================================
 Name        : Ejercicio_P1_Secuencia1.c
 Author      : $Marclé, Emiliano
 Version     : 1.0
 Description :
 Utilizando SysTick, hacer un programa que:
 - Cada vez que se produzca una interrupción por flanco ascendente en P2.2
  saque por el pin P2.1 la secuencia mostrada.
 - En caso de que se produzca una interrupción por flanco descendente en P2.2
  mientras se esta realizando la secuencia, se pondrá en 1 la salida P2.1 y
  se dará por finalizada la secuencia.
  - La entrada debe configurarse con resistencia de pull-down.
  - Suponer cclk = 40 MHz
===============================================================================
*/

#include "LPC17xx.h"

uint8_t sec[16]={0,1,0,1,0,1,0,0,0,0,1,0,1,0,1,0};
volatile uint8_t mostrandoSecuencia = 0;
volatile uint32_t t = 0;
volatile uint32_t t_ref = 0;
uint8_t index = 0;

void iniciarGPIO(void)
{
    //P2.2 COMO ENTRADA E EINT
    LPC_PINCON -> PINSEL4 &= ~(3 << 4);
    LPC_GPIO2 -> FIODIR &= ~(1 << 2);
    LPC_PINCON -> PINMODE4 &= ~(3 << 4);
    LPC_PINCON -> PINMODE4 |= (3 << 4); //PULL DOWN

    //P2.1 COMO SALIDA
    LPC_PINCON -> PINSEL4 &= ~(3 << 2);
    LPC_GPIO2 -> FIODIR |= (1 << 1);
    LPC_GPIO2 -> FIOSET = (1 << 1);
}

void iniciarSysTick(void)
{
    SysTick -> LOAD = (40000000/1000) - 1; //1ms
    SysTick -> VAL = 0;
    SysTick -> CTRL |= 7; 
}

void iniciarEINT3(void)
{
    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_SetPriority(EINT3_IRQn, 1);

    LPC_GPIOINT -> IO2IntEnR |= (1 << 2);
    LPC_GPIOINT -> IO2IntEnF |= (1 << 2);

    LPC_GPIOINT -> IO2IntClr = (1 << 2);
}

void SysTick_Handler(void)
{
    t++;
}

void EINT3_IRQHandler(void)
{
    if(LPC_GPIOINT -> IO2IntStatR & (1 << 2))
    {
        t_ref = t;
        mostrandoSecuencia = 1;

        if(sec[0] == 0)
        {
            LPC_GPIO2 -> FIOCLR = (1 << 1);
        }
        else
        {
            LPC_GPIO2 -> FIOSET = (1 << 1);
        }

        index = 1;
    }
    else if(LPC_GPIOINT -> IO2IntStatF & (1 << 2)) //es mejor que sea un if aparte
    {
        if(mostrandoSecuencia == 1)
        {
            mostrandoSecuencia = 0;
            index = 0;
            LPC_GPIO2 -> FIOSET = (1 << 1);
        }
    }

    LPC_GPIOINT -> IO2IntClr = (1 << 2);
}

int main(void)
{
    iniciarGPIO();
    iniciarSysTick();
    iniciarEINT3();

    while(1)
    {
        if(mostrandoSecuencia)
        {
            if(t - t_ref >= 100) //100ms por intervalo (desicion mia, no lo solicitan)
            {
                if(sec[index] == 0)
                {
                    LPC_GPIO2 -> FIOCLR = (1 << 1);
                }
                else
                {
                    LPC_GPIO2 -> FIOSET = (1 << 1);
                }

                index++;

                if(index >= 16)
                {
                    index = 0;
                    LPC_GPIO2 -> FIOSET = (1 << 1);
                    mostrandoSecuencia = 0;
                }

                t_ref = t;
            }
        }
        else
        {

        }
    }
}
