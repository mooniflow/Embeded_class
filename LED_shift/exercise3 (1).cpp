#define _CRT_SECURE_NO_WARNINGS
#define BIT(X) (1 << X)
#include <stdio.h>

unsigned char cha;
unsigned char LEDs;

int LED_function(int input) {
    LEDs |= BIT(input);
    return LEDs;
}
int count(unsigned char input) {
    int cnt = 0;
    for (int i = 0; i < 8; i++) {
        if ((input & LED_function(i)) > 0) cnt++;
        LEDs = 0;
    }
    return cnt;
}

int main() {

    int temp;
    scanf("%d", &temp);
    
    cha = temp;

    int one_count;
    one_count = count(cha);

    int shift_n = 7;

    for (int i = one_count; i > 0; i--) {
        LED_function(shift_n);
        shift_n--;
    }

    int val = LEDs;

    printf("1의 개수 : %d\n", one_count);
    printf("Shift 시의 값 : %d\n", val);

    return 0;
}