
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>
#include <math.h>

typedef unsigned char uc;
const uc digit[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f };
const uc fnd_sel[4] = { 0x08,0x04,0x02,0x01 };
const uc dot = 0x80;
void main()
{
    int i, t;
    DDRC = 0xff;
    DDRG = 0x0f;
	DDRA = 0xff;
    int num[4];
    int randnum;
    randnum = rand() % 8;
    for(i =0; i<4; i++){
		num[i] = digit[randnum];
	}
	while(1){
		for(t = 0; t < 4; t++) {
		PORTC = num[t];
		PORTG = fnd_sel[t];
		_delay_ms(2);
		}
		PORTA = pow(2, randnum);
	}

	
}
