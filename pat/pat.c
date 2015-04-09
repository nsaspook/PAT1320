#pragma	config	OSC = INTIO2
#pragma config  LVP = OFF
#pragma config	WDT = OFF
/*
 * Driver software for PAT Pic Automation Table project
 * Runs on a PIC18F1320 
 *	nsaspook@nsaspook.com
 * cpu clock 8mhz internal
 * Version
 * 1.1		Read several reed switches, provide audio and visual feedback and send X10 commands to Firecracker device.
 */

#include <p18f1320.h>
#include <timers.h>
#include <stdlib.h>
#include <eep.h>
#include <stdio.h>
#include <string.h>
#include "cm17a.h"
#include "pat.h"

void setup_cm17a(void);
void loop_cm17a(void);
void xmitCM17A_cmd(char, unsigned char, unsigned char);
void xmitCM17A(char, unsigned char, unsigned char, unsigned char);
void tm_handler (void);

#define FALSE	0
#define TRUE	1
#define	ON		0
#define	OFF		1
#define	LEDON	1
#define	LEDOFF	0
#define	DEBOUNCE	50


#define	TIMEROFFSET	34268			// timer0 16bit counter value for 1 second to overflow
#define	TIMERFAST	57724			// fast flash 4hz
#define	SAMPLEFREQ	56000			// timer1 100hz
#define	INITTIME	36
#define	COMMTIME	30
#define	BELL_TIME	100

#define	CODE_HOUSE		'M'
#define	CODE_DEVICE1	11
#define	CODE_DEVICE2	6
#define	CODE_REPEAT		5

volatile unsigned int	timer0_off=TIMEROFFSET;
volatile unsigned char	BUTTON_PRESSED=FALSE,BLINK=FALSE;
volatile unsigned long	button_intcount,sample_intcount,time_intcount, sw_intcount;

const rom char		*build_date = __DATE__, *build_time = __TIME__;

#pragma code tm_interrupt = 0x8
void tm_int (void)
{
        _asm goto tm_handler _endasm
}
#pragma code

#pragma interrupt tm_handler
void tm_handler (void)					// timer functions are handled here
{
static unsigned char	debo=0,bell_count=0;

        if ( PIR1bits.TMR1IF ) {     	//      Timer1 int handler, for switch reading, LED  and X10 controls
                PIR1bits.TMR1IF = 0;   	// clear int flag
                sample_intcount++;
                WriteTimer1 ( SAMPLEFREQ );
//                MPULED=!MPULED;					// flash onboard led (timer1)


// check for mag switch contact
                if (SW1 && SW2 && SW3 && SW4) {									// screen status feedback
                        BUTTON_PRESSED=FALSE;
                        BLINK=FALSE;
						debo=0;
						BELL=LEDOFF;
						bell_count=0;
                } else {
						if (debo++ > DEBOUNCE) {
							sw_intcount++;
//                			MPULED=!MPULED;					// flash onboard led (switch debounce)
							if (bell_count++ < BELL_TIME) {						// sonalert for audio feedback
								BELL=LEDON;
							} else {
								BELL=LEDOFF;
								bell_count=BELL_TIME;
							}
                        	BUTTON_PRESSED=TRUE;
                        	BLINK=TRUE;
							debo=DEBOUNCE;
						}
                }
        }

        if ( INTCONbits.TMR0IF ) {								// check timer0 irq time timer
                //check for TMR0 overflow
                INTCONbits.TMR0IF = 0; 							//clear interrupt flag
                time_intcount++;
                WriteTimer0 ( timer0_off );
                MPULED=!MPULED;					// flash onboard led
                if (BLINK) {
                        if (SW1 == 0) {
                                if (time_intcount%2) 	{
                                        LED1=LEDON;
                                } else {
                                        LED1=LEDOFF;
                                }
                        } 
                        if (SW2 == 0) {
                                if (time_intcount%2) 	{
                                        LED2=LEDON;
                                } else {
                                        LED2=LEDOFF;
                                }
                        }
                        if (SW3 == 0) {
                                if (time_intcount%2) 	{
                                        LED3=LEDON;
                                } else {
                                        LED3=LEDOFF;
                                }
                        }
                        if (SW4 == 0) {
                                if (time_intcount%2) 	{
                                        LED4=LEDON;
                                } else {
                                        LED4=LEDOFF;
                                }
                        }
                } else {
					LED1=LEDON;
					LED2=LEDON;
					LED3=LEDON;
					LED4=LEDON;
				}
        }
}


int	sw_work(void)
{
        static	unsigned char	ONCE=TRUE;

        if (BUTTON_PRESSED) {
              	INTCONbits.GIEH = 0;
             	timer0_off=TIMERFAST;
         		INTCONbits.GIEH = 1;
                if (SW1 == 0) {
                        if (ONCE) {
                        	LED1=LEDOFF;
							xmitCM17A(CODE_HOUSE,CODE_DEVICE1,OFF,CODE_REPEAT);
						}
                        ONCE=FALSE;
                } else {
                        LED1=LEDON;
                }
                if (SW2 == 0) {
                        if (ONCE) {
                        	LED2=LEDOFF;
							xmitCM17A(CODE_HOUSE,CODE_DEVICE2,OFF,CODE_REPEAT);
						}
                        ONCE=FALSE;
                } else {
                        LED2=LEDON;
                }
                if (SW3 == 0) {
                        if (ONCE) {
                        	LED3=LEDOFF;
							xmitCM17A(CODE_HOUSE,CODE_DEVICE2,ON,CODE_REPEAT);
						}
                        ONCE=FALSE;
                } else {
                        LED3=LEDON;
                }
                if (SW4 == 0) {
                        if (ONCE) {
                        	LED4=LEDOFF;
							xmitCM17A(CODE_HOUSE,CODE_DEVICE1,ON,CODE_REPEAT);
						}
                        ONCE=FALSE;
                } else {
                        LED4=LEDON;
                }
        } else {
                ONCE=TRUE;
              	INTCONbits.GIEH = 0;
         		timer0_off=TIMEROFFSET;
         		INTCONbits.GIEH = 1;
        }
        return 0;
}

int	option_switch(void)
{
	if (OPTIONSW) {
		return 0;					// option switch high level, so exit
	} else {
      	INTCONbits.GIEH = 0;		// option switch low level, so process possible startup options
       	timer0_off=TIMERFAST;		// blink fast
       	INTCONbits.GIEH = 1;
/*	Only loop X10 commands for now */
		do {
			loop_cm17a();			// debug, send commands
		} while (!OPTIONSW);
/*	Options	*/
      	INTCONbits.GIEH = 0;
       	timer0_off=TIMEROFFSET;		// blink slow
       	INTCONbits.GIEH = 1;
		return 1;
	}
}

void main (void)
{
        /* Configure  all PORT pins for output */
		ADCON1 = 0x7F;				// all digital, no ADC
        OSCCON = 0x72;				// internal 8mhz
		OSCTUNE =0x00;
        PATPORT = PATPORT_IO;		// [4..7] inputs for mag reed switches
        LEDPORT = LEDPORT_IO;		// [0..7] LED outputs and X10 control pins
		OPTIONSW_IO=1;				// Configure options switch input.

        /* interrupt priority ON */
        RCONbits.IPEN = 1;

		LED1=LEDON;					// light all switch leds
		LED2=LEDON;
		LED3=LEDON;
		LED4=LEDON;

		BELL=LEDOFF;
        MPULED=OFF;					// onboard led OFF

        OpenTimer0 ( TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_64 );
        WriteTimer0 ( timer0_off );						//	start timer0 at 1 second ticks
        OpenTimer1 ( TIMER_INT_ON & T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_1 & T1_OSC1EN_OFF & T1_SYNC_EXT_OFF );
        WriteTimer1 ( SAMPLEFREQ );						// about 250hz

        /*      work int thread setup */
        INTCONbits.TMR0IE=1;			// enable int
		INTCON2bits.TMR0IP=1;			// make it high level
        PIE1bits.TMR1IE = 1;          	// enable int
        IPR1bits.TMR1IP = 1;          	// make it high level
        /* Enable all high priority interrupts */
        INTCONbits.GIEH = 1;
        setup_cm17a();							// config X10 ports

		option_switch();				// see if we need to change options.

        /* Loop forever */
        while (TRUE) {							// busy work
                sw_work();						// read switches and send X10 commands
        }
}
