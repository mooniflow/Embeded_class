
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/interrupt.h>

typedef unsigned char uc;
const uc digit[10] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x27, 0x7f, 0x6f };
const uc fnd_sel[4] = { 0x08,0x04,0x02,0x01 };
const uc dot = 0x80;
void main()
{
   int i, j, k, l, t;
   DDRC = 0xff;
   DDRG = 0x0f;
   int num[4];
   for (i = 0; ; i++) {
      if (i == 10)
         i = 0;

      num[0] = digit[i];
      for (j = 0; j < 10; j++) {
         num[1] = digit[j] + dot;
         for (k = 0; k < 10; k++) {
            num[2] = digit[k];
            for (l = 0; l < 10; l++) {
               num[3] = digit[l];
               
               for(t = 0; t < 4; t++) {
                  PORTC = num[t];
                  PORTG = fnd_sel[t];
                  _delay_ms(2.5);
               }
            }
         }
      }
   }
}
