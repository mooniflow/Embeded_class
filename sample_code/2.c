#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
unsigned char LEDs;

int LED_function(int input){
    /*
    * 
    your code

    */
}

int main() {
    int temp;
    scanf("%d", &temp); //0~7숫자 입력
    int val = LED_function(temp);
    printf("return value : %d\n", val);
    return 0;
}
