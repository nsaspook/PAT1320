//	PIC CM17A Firecracker interface Version 1.1
//	hardware defines, needs 3 bits, two for X10 module and one for signal device and 4 for display leds
//  Configure all 8 bits for output for LED display

#define CM17A_PORT_OUT	TRISB
#define CM17A_PINS		0b00000000		// PORT output pins
#define RTS_pin 		LATBbits.LATB0	// RTS line for CM17A - PORTx pin 0
#define DTR_pin 		LATBbits.LATB1	// DTR line for CM17A - PORTx pin 1
#define BELL 			LATBbits.LATB2	// sonalert/buzzer output - PORTx pin 2, HIGH for sound
#define	X10_DELAY		3				// cpu cycles delay for X10 bit spacing
