// -----------------------------------------------------------------------------
// ------------------- INTERFAZ DEL MÓDULO DE CONFIGURACIÓN --------------------
// -----------------------------------------------------------------------------

/**
 * @file     config.h
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

#ifndef _CONFIG_H
#define _CONFIG_H

// -----------------------------------------------------------------------------
// -------------------------------- PARÁMETROS ---------------------------------
// -----------------------------------------------------------------------------

/// Frecuencia de trabajo del microprocesador (Hz). El UL final indica al
/// compilador que este número debe operarse como un unsigned long.
#define FCY 39613750UL

// -----------------------------------------------------------------------------
// ------------------- PROTOTIPOS DE LAS FUNCIONES PÚBLICAS --------------------
// -----------------------------------------------------------------------------

/**
 * Inicializa el reloj interno FRC con PLL.
 *
 * Configura la frecuencia del oscilador FRC (FOSC), cuya frecuencia nominal
 * (Fin) son 7.37 MHz, para que el microprocesador opere a 40 MIPS (FCY).
 * FOSC = Fin * M/(N1*N2)   FCY = FOSC/2
 * FOSC = 79.2275 MHz       FCY = 39.61375 MHz
 */
void inicializarReloj(void);

#endif
