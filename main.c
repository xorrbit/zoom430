#include <msp430.h>

#include <stdlib.h>

#include "pcd8544.h"

void zoom430(void)
{
	int pos = 5;
	int l[6];
	int r[6];

	int i;
	int rando;

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
			l[5]++;
		} else if (rando == 1) {
			l[5]--;
		}
				
		if (l[5] < 0) {
		       l[5] = 0;
		}

		if (l[5] > (r[5] - 4)) {
			l[5] = r[5] - 4;
		}

		rando = rand() % 3;
		if (rando == 0) {
			r[5]++;
		} else if (rando == 1) {
			r[5]--;
		}
				
		if (r[5] > 11) {
		       r[5] = 11;
		}

		if (r[5] < (l[5] + 4)) {
			r[5] = l[5] + 4;
		}

		LCD_gotoXY(l[5] * 7, 5);
		LCD_writeString("|");
		LCD_gotoXY(r[5] * 7, 5);
		LCD_writeString("|");

		// check butons, adjust pos
	
		// draw ship
		LCD_gotoXY(pos * 7, 5);
		LCD_writeString("V");	

		// coldet
	
		// delay
		__delay_cycles(2000000);

	}
}

int main(void)
{
	WDTCTL = WDTPW + WDTHOLD; // stop watchdog

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

	zoom430();

	for(;;);
}

