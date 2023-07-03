#define _CRT_SECURE_NO_WARNINGS
#define BIT(X) (1 << X)
#include <stdio.h>

unsigned char LEDs;

int LED_function(int input) {
    LEDs |= BIT(input);
    return LEDs;
}

int main() {
    
    int val;
    int cnt = 0;

    while (1) {

        val = LED_function(0);
        printf("return value : %d\n", val);
        LEDs = 0;

        for (int i = 1; i < 8; i++) {

            val = LED_function(i);
            printf("return value : %d\n", val);

            LEDs = 0;

        }

        for (int i = 1; i < 7; i++) {

            val = LED_function(7-i);
            printf("return value : %d\n", val);

            LEDs = 0;

        }


        cnt++;
        if (cnt == 4) {
            val = LED_function(0);
            printf("return value : %d\n", val);
            break;
        }
    }

    return 0;
}
