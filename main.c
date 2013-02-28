#include <msp430.h>

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "pcd8544.h"

volatile bool R_PRESSED = false;
volatile bool L_PRESSED = false;

int zoom430(void)
{
	int pos = 5;
	int l[6];
	int r[6];
	int dist = 0;
	char str[10];

	int i;
	int rando;

	bool R_WAS_PRESSED = false;
	bool L_WAS_PRESSED = false;

	// set up and draw initial screen
	for (i = 0; i < 6; i++) {
		l[i] = pos - 3;
		LCD_gotoXY(l[i] * 7, i);
		LCD_writeString("|");

		r[i] = pos + 3;
		LCD_gotoXY(r[i] * 7, i);
		LCD_writeString("|");
	}

	// draw ship
	LCD_gotoXY(pos * 7, 5);
	LCD_writeString("V");

	// compensates for L_PRESSED :)
	pos++;
	// wait until you press a button
	while (!L_PRESSED);

	// main loop
	for (;;) {
		// erase ship
		LCD_gotoXY(pos * 7, 5);
		LCD_writeString(" ");

		// erase and draw walls
		for (i = 0; i < 6; i++) {
			LCD_gotoXY(l[i] * 7, i);
			LCD_writeString(" ");
			LCD_gotoXY(r[i] * 7, i);
			LCD_writeString(" ");

			if (i < 5) {
				l[i] = l[i+1];
				r[i] = r[i+1];
				
				LCD_gotoXY(l[i] * 7, i);
				LCD_writeString("|");
				LCD_gotoXY(r[i] * 7, i);
				LCD_writeString("|");
			}
		}

		// new row!
		rando = rand() % 3;
		if (rando == 0) {
			if (l[5] < (r[5] - 4)) {
				l[5]++;
			} else if (l[5] > 0) {
				l[5]--;
			}
		} else if (rando == 1) {
			if (l[5] > 0) {
				l[5]--;
			} else if (l[5] < (r[5] - 4)) {
				l[5]++;
			}
		}
		
		rando = rand() % 3;
		if (rando == 0) {
			if (r[5] > (l[5] + 4)) {
				r[5]--;
			} else if (r[5] < 11) {
				r[5]++;
			}
		} else if (rando == 1) {
			if (r[5] < 11) {
				r[5]++;
			} else if (r[5] > (l[5] + 4)) {
				r[5]--;
			}
		}
				
		LCD_gotoXY(l[5] * 7, 5);
		LCD_writeString("|");
		LCD_gotoXY(r[5] * 7, 5);
		LCD_writeString("|");

		// check butons, adjust pos

		if (L_PRESSED) {
			if (!L_WAS_PRESSED) {
				pos--;
				if (pos < 0) {
					pos = 0;
				}
			}
			L_WAS_PRESSED = true;
		} else {
			L_WAS_PRESSED = false;
		}

		if (R_PRESSED) {
			if (!R_WAS_PRESSED) {
				pos++;
				if (pos > 11) {
					pos = 11;
				}
			}
			R_WAS_PRESSED = true;
		} else {
			R_WAS_PRESSED = false;
		}

		// draw ship
		LCD_gotoXY(pos * 7, 5);
		LCD_writeString("V");	

		// coldet

		if (pos <= l[5] || pos >= r[5]) {
			// boom
			return dist;
		}

		LCD_gotoXY(0, 0);
		sprintf(str, "%d", dist);
		LCD_writeString(str);

		dist++;

		// delay
		__delay_cycles(3000000);

	}
}

int main(void)
{
	int dist = 0;
	char str[20];
	// set up watchdog timer to debounce NMI/RESET button
	WDTCTL = WDTPW | WDTHOLD | WDTNMIES | WDTNMI;
	
	P1DIR &= ~BIT3; // p1.3 is input
	P1OUT |= BIT3; // p1.3 has internal pullup
	P1REN |= BIT3;
	P1IES |= BIT3; // falling edge
	P1IE |= BIT3; // enable interrupt
	
	IFG1 &= ~(P1IFG | WDTIFG | NMIIFG); // clear watchdog and nmi interrupts
	IE1 |= P1IE | WDTIE | NMIIE; // enable watchdog and nmi interrupts

	__bis_SR_register(GIE); // enable global interrupts 

	LCD_init();

	LCD_gotoXY(0, 0);

	if (CALBC1_16MHZ == 0xff) {
		// no calibration values for 16 MHz
		LCD_writeString("No 16 MHz calibration constants!");
		for(;;);
	}

	DCOCTL = 0;
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;

	srand(42); // eh

	for(;;) {
		LCD_clear();
		
		dist = zoom430();
		
		sprintf(str, "LOSE at %d!", dist);
		LCD_gotoXY(0,0);
		LCD_writeString(str);

		__delay_cycles(16000000);
		
		while(L_PRESSED);
		while(!L_PRESSED);
	}
}

#pragma vector = NMI_VECTOR
__interrupt void nmi_isr(void)
{
	if (IFG1 & NMIIFG) { // nmi caused by nRST/NMI pin
		IFG1 &= ~NMIIFG; // clear it
		if (WDTCTL & WDTNMIES) { // falling edge detected
			R_PRESSED = true; // tell the rest of the world that s1 is depressed
			WDTCTL = WDT_MDLY_32 | WDTNMI; // debounce and detect rising edge
		} else { // rising edge
			R_PRESSED = false; // tell the rest of the world that s1 is released
			WDTCTL = WDT_MDLY_32 | WDTNMIES | WDTNMI; // debounce and detect falling edge
		}
	} // Note that NMIIE is cleared; wdt_isr will set NMIIE 32msec later
}

#pragma vector = WDT_VECTOR
__interrupt void wdt_isr(void)
{
	WDTCTL ^= (0x3300 | WDTHOLD); // flip wdthold while keeping other bits in tact
	IFG1 &= ~NMIIFG; // It may have been set by switch bouncing, clear it
	IE1 |= NMIIE; // Now we can enable nmi to detect the next edge
}

#pragma vector = PORT1_VECTOR
__interrupt void port1_isr(void)
{
	if (P1IFG & BIT3) { // p1.3
		if (P1IES & BIT3) {
			// falling edge
			L_PRESSED = true;
			P1IES ^= BIT3; // toggle edge
		} else {
			// rising edge
			L_PRESSED = false;
			P1IES ^= BIT3;
		}
		P1IFG &= ~BIT3;
	}
}

