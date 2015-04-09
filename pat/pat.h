//	Pic Automation Table Version 1.1
//	hardware defines
#include "cm17a.h"

#define PATPORT		TRISA			// Port to read switches and drive feedback leds
#define PATPORT_IO	0b00001111		// [0..3] inputs for mag reed switches

#define	SWPORT		PORTA			// button is LOW when pushed or activated
#define SW1			PORTAbits.RA0
#define SW2			PORTAbits.RA1
#define SW3			PORTAbits.RA2
#define SW4			PORTAbits.RA3

#define	LEDPORT		CM17A_PORT_OUT	// led port output is HIGH when lit
#define	LEDPORT_IO	CM17A_PINS		// Use CM17A port for LEDS
#define LED1		LATBbits.LATB4
#define LED2		LATBbits.LATB5
#define LED3		LATBbits.LATB6
#define LED4		LATBbits.LATB7
#define	MPULED		LATBbits.LATB3

#define	OPTIONSW	PORTAbits.RA4
#define	OPTIONSW_IO	TRISAbits.TRISA4

#define	MISC_A6		LATAbits.LATA6
#define	MISC_A7		LATAbits.LATA7