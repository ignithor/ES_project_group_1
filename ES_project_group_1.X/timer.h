/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

#ifndef XC_HEADER_TEMPLATE_H
#define XC_HEADER_TEMPLATE_H

#include <xc.h> // Include processor files

// Timer usage definitions:
// TIMER1: Main loop period management at 100Hz.
// TIMER2: Busy wait for 7ms within algorithm execution.
#define TIMER1 1
#define TIMER2 2

// Define System Clock Frequency used for timer calculations.
#define FCY 72000000

// Function Prototypes:

/**
 * @brief Configure the specified timer period.
 * 
 * Sets up the timer to generate a period flag after a specified duration in milliseconds.
 *
 * @param timer Timer identifier (TIMER1, TIMER2).
 * @param ms Time period in milliseconds.
 */
void tmr_setup_period(int timer, int ms);

/**
 * @brief Wait until the specified timer period is complete.
 * 
 * Blocks the execution until the timer flag indicates the period is finished.
 *
 * @param timer Timer identifier.
 */
void tmr_wait_period(int timer);

/**
 * @brief Busy wait for a specified duration.
 * 
 * Uses the selected timer to perform a busy wait for the given number of milliseconds.
 *
 * @param timer Timer identifier.
 * @param ms Number of milliseconds to wait.
 */
void tmr_wait_ms(int timer, int ms);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */