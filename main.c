/*
 * File:   main.c
 * Author: fjb29
 *
 * Created on 28 de agosto de 2021, 16:17
 */


#include "p33FJ32MC202.h"
#include "config.h"
 
//b

#define LED12 PORTBbits.RB12
#define LED13 PORTBbits.RB13
#define LED14 PORTBbits.RB14 
#define LED15 PORTBbits.RB15
#define SW2 PORTBbits.RB2

void InicializaES(void);
void InicializaTimer1(void);
void InicializaINT1(void);
void InicializaPWM1(void);

int CalculaPrecargaTimer(float ms);
void updateDuty(void);

static unsigned int local_timer=0;
static _Bool flag_int_T1A = 0; 
static _Bool flag_int_T1B = 0; 
static int precarga_PTPER = 0;
static float duty_cycle = 0.5;
 

int main(void) 
{
    inicializarReloj();
    InicializaES(); 
    InicializaTimer1();
    InicializaINT1();
    InicializaPWM1();
   
    while(1)
    {
        /*
        //timer 1 sin interrupciones
        if (IFS0bits.T1IF)
        {
            IFS0bits.T1IF = 0;
            local_timer++;
            
            if (local_timer==10)
            {
                local_timer = 0;
                LED15 = ~LED15;
            }
        }
        */
      
        /*
        // entrada 2 sin interrupciones
        if ( SW2 )
            LED14 = 1;    
        else
            LED14 = 0;
        */
          
        
        //cada 200 interrupciones del timer 1...
        if (flag_int_T1A)
        {
           flag_int_T1A = 0;            
           LED12 = ~LED12;
        }
        
        //cada interrupción del timer 1...
        if (flag_int_T1B)
        {
            flag_int_T1B = 0;
            updateDuty();
        }
        
        
        
    };
    return 0;
}


void __attribute__((interrupt, no_auto_psv))_INT1Interrupt(void)
{    
    LED14 = ~LED14;
    IFS1bits.INT1IF = 0;
}

void __attribute__((interrupt, no_auto_psv))_T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    
    local_timer++;
    
    if (local_timer == 200)
    {
        local_timer = 0;
        flag_int_T1A = 1;
    }
    flag_int_T1B = 1;
    
}


void InicializaES(void)
{
    AD1PCFGL = 0xFFFF;  //no olvidar si se van a trabajar con los pines superiores
    
    TRISB &= ~(1<<15);      //Bit 15 salida
    TRISB &= ~(1<<14);      //Bit 14 salida
    
    TRISB &= ~(1<<12);      //Bit 12 salida
    
    TRISBbits.TRISB2 = 1;   //Bit 2 entrada
    
    PORTB |= 1<<15;         //a cero
    PORTB |= 1<<14;         //a cero
    
    PORTB |= 1<<12;         //a cero
    
}

void InicializaINT1(void)
{
    INTCON2bits.INT1EP = 1;  //positive edge
    IFS1bits.INT1IF = 0;     //clear flag
    IEC1bits.INT1IE = 1;     //interrupt request enabled
    IPC5bits.INT1IP = 7;     //maxima prioridad
    RPINR0bits.INT1R = 2;    //interrupción en pin RP2     
}

void InicializaTimer1(void)
{
    T1CON  = 0x0030; //divisor frecuencia
    //PR1    = 1250;   //prescala
    PR1 = CalculaPrecargaTimer(1);
    
    T1CON |= 0x8000; //timer ON    
    
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    IPC0bits.T1IP = 1;
}
 
void InicializaPWM1(void)
{
     float time_base = 0.001;  //1 milisegundo periodo base del PWM

    P1TCONbits.PTOPS  = 0;     //postscale = 0
    P1TCONbits.PTCKPS = 2;     //prescale  = 1/16
    //P1TCONbits.PTMOD  = 2 ;    //PWM time base operates in a Continuous Up/Down Count mode
    P1TCONbits.PTMOD  = 0 ;    // PWM time base operates in a Free-Running mode

    PWM1CON1bits.PMOD1 = 1; //independent PWM Output mode
    PWM1CON1bits.PEN1L = 1; //PWM output in PWML
    PWM1CON1bits.PEN1H = 0; //PWM not in PWMH
   
    P1TMR = 0;
    
    P1TMRbits.PTDIR = 0;   //dirección ascendiente
    
    precarga_PTPER = time_base*FCY/16 - 1;
        
    PTPER = precarga_PTPER;
    P1DC1 = 0.5*(precarga_PTPER+1)*2;   
    
    PTCONbits.PTEN = 1;       //start PWM
    
}
 
int CalculaPrecargaTimer(float ms)
{
    //habría que hacer un switch - case con las opciones de inicialización
    int divisor_frecuencia = 256;
        
    float fosc = FCY/divisor_frecuencia;
    
    float contaje = 0.001*ms*fosc;
    
    if ( contaje > 32768.0 )
        return 2^15;
    else
        return (int)(0.001*ms*fosc);
    
}

void updateDuty(void)
{
    /* 
    //en función del estado del led
    if (LED14==0)
       duty_cycle -= 0.02;
           
    if (LED14==1)
       duty_cycle += 0.02;
           
    if (duty_cycle > 1.0)
       duty_cycle = 1.0;

    if (duty_cycle < 0.0)
       duty_cycle = 0.0;
    */
    duty_cycle += 0.0005;
    if (duty_cycle > 1.0)
            duty_cycle = 0.0;
    
    P1DC1 = (int) (duty_cycle*(precarga_PTPER+1)*2);  
}