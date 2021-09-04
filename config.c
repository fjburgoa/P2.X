// -----------------------------------------------------------------------------
// ------------------------- MÓDULO DE CONFIGURACIÓN ---------------------------
// -----------------------------------------------------------------------------

/**
 * @file     config.c
 *
 * @author   Jaime Boal Martín-Larrauri
 *
 * @version  2.1.0
 *
 * @date     28/08/2018
 *
 * @brief    Funciones de configuración del microcontrolador.
 */

// -----------------------------------------------------------------------------

#include <xc.h>  // Incluye el microcontrolador seleccionado en el proyecto
#include "config.h"

// -----------------------------------------------------------------------------
// --------------------------- BITS DE CONFIGURACIÓN ---------------------------
// -----------------------------------------------------------------------------

// FBS
#pragma config BWRP = WRPROTECT_OFF  // Permitir la escritura del segmento de arranque flash
#pragma config BSS = NO_FLASH        // Eliminar el segmento de arranque flash

// FGS
#pragma config GWRP = OFF            // No proteger la memoria de programa contra escritura
#pragma config GSS = OFF             // No proteger el código

// FOSCSEL
#pragma config FNOSC = FRC           // Utilizar el oscilador interno (FRC) en el arranque
#pragma config IESO = OFF            // Arrancar directamente con el oscilador seleccionado

// FOSC
#pragma config POSCMD = NONE         // Desactivar el oscilador primario
#pragma config OSCIOFNC = ON         // Utilizar el oscilador secundario como entrada y salida digital
#pragma config IOL1WAY = OFF         // Permitir múltiples remapeos de los pines
#pragma config FCKSM = CSECMD        // Permitir la conmutación del reloj y deshabilitar el control de fallos

// FWDT
#pragma config FWDTEN = OFF          // Watchdog timer controlado por software (Inicialmente deshabilitado)
#pragma config WDTPOST = PS32768     // Postescalado del watchdog (1:32,768)
#pragma config WDTPRE = PR128        // Preescalado del watchdog (1:128)
#pragma config WINDIS = OFF          // Permitir resetear el watchdog en cualquier momento

// FPOR
#pragma config FPWRT = PWR128        // Esperar 128 ms y resetear el microcontrolador al enchufar la alimentación
#pragma config ALTI2C = OFF          // Utilizar los pines estándar (SDA1 y SCL1) para el I2C
#pragma config LPOL = ON             // Los pines PWM low están activos a nivel alto
#pragma config HPOL = ON             // Los pines PWM high están activos a nivel alto
#pragma config PWMPIN = ON           // Controlar los pines de PWM desde el registro PORTx al arrancar

// FICD
#pragma config ICS = PGD1            // Programar y depurar a través de los pines PGEC1 y PGED1
#pragma config JTAGEN = OFF          // Desactivar la interfaz para JTAG

// -----------------------------------------------------------------------------
// ---------------------------- FUNCIONES PÚBLICAS -----------------------------
// -----------------------------------------------------------------------------

void inicializarReloj(void) {
    CLKDIVbits.PLLPRE  =  0;  // Preescalado del PLL:   N1 = 2
    PLLFBD             = 41;  // Multiplicador del PLL: M = PLLFBD + 2 = 43
    CLKDIVbits.PLLPOST =  0;  // Postescalado del PLL:  N2 = 2

    // Funciones para desbloquear la escritura del registro OSCCON
    __builtin_write_OSCCONH(0x01);           // Nuevo reloj: FRC w/ PLL
    __builtin_write_OSCCONL(OSCCON | 0x01);  // Iniciar el cambio de reloj

    while (OSCCONbits.COSC != 1);  // Esperar al cambio de reloj
    while (OSCCONbits.LOCK != 1);  // Esperar a que se sincronice el PLL
}
