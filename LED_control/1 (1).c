#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

void main()
{
	unsigned char value = 128;
	DDRA = 0xff;
	for(;;){
		for(;;){
			PORTA = value;
			_delay_ms(200);
			value >>= 1;
			if(value == 0) {
				value = 1;
				break;
			}
		}
		for(;;){
			PORTA = value;
			_delay_ms(200);
			value <<= 1;
			if(value == 256){ 
				value = 128;
				break;
			}
		}
	}
}
