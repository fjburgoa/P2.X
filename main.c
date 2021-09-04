/*
 * File:   main.c
 * Author: fjb29
 *
 * Created on 28 de agosto de 2021, 16:17
 */


#include "p33FJ32MC202.h"
#include "config.h"
#include <stdio.h>
 

#define LED12 PORTBbits.RB12
#define LED13 PORTBbits.RB13
#define LED14 PORTBbits.RB14 
#define LED15 PORTBbits.RB15
#define SW2 PORTBbits.RB2

void InicializaES(void);
void InicializaTimer1(void);
void InicializaINT1(void);
void InicializaPWM1(void);
void InicializaUART(unsigned long baudrate);
void InicializaADC(float sample_time);


int CalculaPrecargaTimer(float ms);
void updateDuty(void);

static unsigned int local_timer=0;
static _Bool flag_int_T1A = 0; 
static _Bool flag_int_T1B = 0; 

static float duty_cycle = 0.5;
static char dato_recibido = 0;
static unsigned int contador = 0;
static int precarga_PTPER = 0;   //cálculo de la precarga del timer 
static int medida_adc = 0;
 

int main(void) 
{
    inicializarReloj();
    InicializaES(); 
    InicializaTimer1();
    InicializaINT1();
    InicializaPWM1();
    InicializaUART(9600);
    InicializaADC(2000.0); //periodo de muestreo = 2kHz
   
    while(1)
    {
        //cada 200 interrupciones del timer 1... ~2.5Hz
        if (flag_int_T1A)
        {
           flag_int_T1A = 0;            
           LED12 = ~LED12;           //led 12 (RB12) encendido apagado por temporizador lento de T1
           printf("T: %d, %d \r\n",contador++, medida_adc);
        }
        
        //cada interrupción del timer 1...
        if (flag_int_T1B)
        {
            flag_int_T1B = 0;        //efecto dimming en RB15 con la salida PWM
            updateDuty();             
        }        
    };
    return 0;
}

/******************** INTERRUPCION EXTERNA 1 *******************************************/
void __attribute__((interrupt, no_auto_psv))_INT1Interrupt(void)
{    
    LED14 = ~LED14;             //cuando se recibe interrupción por flanco descendente, cambia de 
    IFS1bits.INT1IF = 0;
}

/******************** INTERRUPCION TIMER1 **********************************************/
void __attribute__((interrupt, no_auto_psv))_T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    
    local_timer++;
    
    if (local_timer == 200)  
    {
        local_timer = 0;
        flag_int_T1A = 1;     //flag de 200 interrupciones
    }
    flag_int_T1B = 1;         //flag de interrupción 
}


/******************** INTERRUPCION UART ************************************************/
void __attribute__((interrupt, no_auto_psv))_U1RXInterrupt(void) 
{
    dato_recibido = U1RXREG;
    
    if ( U1STAbits.OERR )
        U1STAbits.OERR = 0;

    IFS0bits.U1RXIF = 0; // Borrar la bandera
    
    LED13 = (dato_recibido % 2 == 0) ? 1 : 0;

}

void __attribute__((interrupt, no_auto_psv))_ADC1Interrupt(void) 
{
    IFS0bits.AD1IF = 0; // Borrar la bandera
    
    medida_adc = ADC1BUF0;
    
    //AD1CHS0 = pin_ad; // Seleccionar el nuevo pin AD

    AD1CON1bits.SAMP = 1;  ; // Empezar a muestrear
}


/***************************************************************************************/
/******************** INICIALIZACIONES DE PERIFERICOS  *********************************/
/***************************************************************************************/

void InicializaADC(float sample_time)
{
    AD1PCFGL &= ~(1<<5);   //AN5 as analog input
    AD1CHS0  |= 5;         //Chanel 5 is selected (ADC is multiplexed)
       
    
    AD1CON3bits.ADCS = (int)((sample_time*FCY)) - 1;
    
    AD1CON1bits.ADON   = 1; //ADC module is ON
    AD1CON1bits.AD12B  = 1; //12 bits
    AD1CON1bits.FORM   = 0; //salida en forma de enteros
    AD1CON1bits.SSRC   = 7; //Internal counter ends sampling and starts conversion (auto-convert)
    AD1CON1bits.SIMSAM = 0; //Samples multiple channels individually in sequence
    AD1CON1bits.ASAM   = 0; //Sampling begins when SAMP bit is set
    AD1CON1bits.SAMP   = 1; //ADC sample-and-hold amplifiers are holding
    
    //AD1CON1 = 0x80E0; // ON, conversión automática
    //AD1CON1 |= 0x0001; // Empezar a muestrear
    
    IFS0bits.AD1IF = 0; // Borrar la bandera
    IEC0bits.AD1IE = 1; // Habilitar interrupciones
    IPC3bits.AD1IP = 4; // Prioridad interrupciones

}




/***************************************************************************************/
void InicializaUART(unsigned long baudrate)
{
    __builtin_write_OSCCONL(OSCCON & 0xBF); // Desbloquear el PPS
    RPINR18bits.U1RXR = 5;                  // Remapear U1RX a RP5
    RPOR2bits.RP4R    = 3;                  // Remapear U1TX a RP4
    __builtin_write_OSCCONL(OSCCON | 0x40); // Bloquear el PPS

    U1BRG  = FCY / baudrate / 16 - 1; // Velocidad
    U1MODE = 0x0000; // 8N1
    U1STA  = 0x8000; // Configuración de interrupciones

    IFS0bits.U1RXIF = 0; // Borrar la bandera Rx
    IFS0bits.U1TXIF = 0; // Borrar la bandera Tx
    IEC0bits.U1RXIE = 1; // Habilitar interrupción Rx
    IEC0bits.U1TXIE = 0; // Deshabilitar interrupción Tx
    IPC2bits.U1RXIP = 6; // Prioridad interrupción Rx
    //IPC3bits.U1TXIP = 5; // Prioridad interrupción Tx

    U1MODE |= 1 << 15; // Encender la UART
    U1STA |= 1 << 10; // Habilitar el módulo Tx
}


/***************************************************************************************/
void InicializaES(void)
{
    AD1PCFGL = 0xFFFF;  //no olvidar si se van a trabajar con los pines superiores
    
    //OUTPUTS
    TRISB &= ~(1<<15);      //Bit 15 salida
    TRISB &= ~(1<<14);      //Bit 14 salida
    TRISB &= ~(1<<13);      //Bit 14 salida
    TRISB &= ~(1<<12);      //Bit 12 salida

    PORTB |= 1<<15;         //a cero
    PORTB |= 1<<14;         //a cero    
    PORTB |= 1<<13;         //a cero    
    PORTB |= 1<<12;         //a cero
    
    //INT1
    TRISBbits.TRISB2 = 1;   //Bit 2 entrada --> además es la interrupción en RB2
    
    //UART
    TRISB |= 1 << 5;        // U1RX (RB5) - Entrada
    TRISB &= ~(1 << 4);     // U1TX (RB4) - Salida
    
    //ADC
    TRISBbits.TRISB3 = 1;        // RB3 como input (analógica)
}

/***************************************************************************************/
void InicializaINT1(void)
{
    INTCON2bits.INT1EP = 1;  //positive edge
    IFS1bits.INT1IF = 0;     //clear flag
    IEC1bits.INT1IE = 1;     //interrupt request enabled
    IPC5bits.INT1IP = 7;     //maxima prioridad
    RPINR0bits.INT1R = 2;    //reasigna la interrupción en pin RP2     
}

/***************************************************************************************/
void InicializaTimer1(void)
{
    T1CON  = 0x0030; //divisor frecuencia
    //PR1    = 1250;   //prescala
    PR1 = CalculaPrecargaTimer(1);   //preescala = 1 milisegundo
    
    T1CON |= 0x8000; //timer ON    
    
    IFS0bits.T1IF = 0;   //borra flag
    IEC0bits.T1IE = 1;   //habilita interrupción
    IPC0bits.T1IP = 1;   //prioridad baja
}

/***************************************************************************************/
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
 
/***************************************************************************************/
/***************************************************************************************/
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