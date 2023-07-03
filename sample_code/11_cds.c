#include <avr/io.h>
#define CDS_VALUE 871
void init_adc();
unsigned short read_adc();
void show_adc(unsigned short value);

char fnd_value[3] = {};

int main() {
	unsigned short value;
	DDRA = 0xff;
	init_adc();
	while (1) {
		value = read_adc();
		show_adc(value);
	}
}

void init_adc() {
	ADMUX = 0x00;
	// 00000000
	// REFS(1:0) = "00" AREF(+5V) 기준전압 사용
	// ADLAR = '0' 디폴트 오른쪽 정렬
	// MUX(4:0) = "00000" ADC0 사용, 단극 입력
	ADCSRA = 0x87;
	// 10000111
	// ADEN = '1' ADC를 Enable
	// ADFR = '0' single conversion 모드
	// ADPS(2:0) = "111" 프리스케일러 128분주
}

unsigned short read_adc() {
	unsigned char adc_low, adc_high;
	unsigned short value;
	ADCSRA |= 0x40; // ADC start conversion, ADSC = '1'
	while ((ADCSRA & (0x10)) != 0x10); // ADC 변환 완료 검사
	adc_low = ADCL;
	adc_high = ADCH;
	value = (adc_high << 8) | adc_low;

	return value;
}

void show_adc(unsigned short value) {
	if (value < CDS_VALUE) {
		PORTA = 0xff;

	}
	else PORTA = 0x00;
}