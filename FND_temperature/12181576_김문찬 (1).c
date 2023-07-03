#include "includes.h"
#define F_CPU   16000000UL   // CPU frequency = 16 Mhz
#define F_SCK 40000UL  // SCK 클록 값 = 40 Khz
#include <avr/io.h>
#include <util/delay.h>
#define  TASK_STK_SIZE  OS_TASK_DEF_STK_SIZE
#define  N_TASKS        3

#define UCHAR unsigned char // UCHAR 정의
#define USHORT unsigned short // USHORT 정의
#define ATS75_ADDR 0x98 // 0b10011000, 7비트를 1비트 left shift
#define ATS75_CONFIG_REG 1
#define ATS75_TEMP_REG 0

OS_STK       TaskStk[N_TASKS][TASK_STK_SIZE];
OS_EVENT* Mbox;
OS_EVENT* Sem;

volatile INT8U   fvalue;
   
void  TemperatureTask(void* data);
void  FndTask(void* data);
void  FndDisplayTask(void* data);
void write_twi_1byte_nopreset(UCHAR reg, UCHAR data);
void write_twi_0byte_nopreset(UCHAR reg);


int main(void)
{
    OSInit();
    OS_ENTER_CRITICAL();
    TCCR0 = 0x07;
    TIMSK = _BV(TOIE0);
    TCNT0 = 256 - (CPU_CLOCK_HZ / OS_TICKS_PER_SEC / 1024);
    OS_EXIT_CRITICAL();

    Mbox = OSMboxCreate((void*)0);

    OSTaskCreate(TemperatureTask, (void*)0, (void*)&TaskStk[0][TASK_STK_SIZE - 1], 0);
    OSTaskCreate(FndTask, (void*)0, (void*)&TaskStk[1][TASK_STK_SIZE - 1], 1);
    OSTaskCreate(FndDisplayTask, (void*)0, (void*)&TaskStk[2][TASK_STK_SIZE - 1], 2);

    OSStart();

    return 0;
}

void InitI2C()
{
    DDRC = 0xff; DDRG = 0xff;
    PORTD = 3;                   // For Pull-up override value
    SFIOR &= ~(1 << PUD);          // PUD
    TWSR = 0;                   // TWPS0 = 0, TWPS1 = 0
    TWBR = 32;                  // for 100  K Hz bus clock
    TWCR = _BV(TWEA) | _BV(TWEN);   // TWEA = Ack pulse is generated
    // TWEN = TWI 동작을 가능하게 한다*/
}

void write_twi_1byte_nopreset(UCHAR reg, UCHAR data)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // START 전송
    while (((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10)); // ACK를 기다림
    TWDR = ATS75_ADDR | 0;  // SLA+W 준비, W=0
    TWCR = (1 << TWINT) | (1 << TWEN);  // SLA+W 전송
    while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18);
    TWDR = reg;    // aTS75 Reg 값 준비
    TWCR = (1 << TWINT) | (1 << TWEN);  // aTS75 Reg 값 전송
    while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28);
    TWDR = data;    // DATA 준비
    TWCR = (1 << TWINT) | (1 << TWEN);  // DATA 전송
    while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28);
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // STOP 전송
}

void write_twi_0byte_nopreset(UCHAR reg)
{
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // START 전송
    while (((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10));  // ACK를 기다림
    TWDR = ATS75_ADDR | 0; // SLA+W 준비, W=0
    TWCR = (1 << TWINT) | (1 << TWEN);  // SLA+W 전송
    while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18);
    TWDR = reg;    // aTS75 Reg 값 준비
    TWCR = (1 << TWINT) | (1 << TWEN);  // aTS75 Reg 값 전송
    while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28);
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // STOP 전송
}

INT8U ReadTemperature(void)
{
    int value;

    TWCR = _BV(TWSTA) | _BV(TWINT) | _BV(TWEN);
    while (!(TWCR & _BV(TWINT)));

    TWDR = 0x98 + 1; //TEMP_I2C_ADDR + 1
    TWCR = _BV(TWINT) | _BV(TWEN);
    while (!(TWCR & _BV(TWINT)));

    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
    while (!(TWCR & _BV(TWINT)));

    //온도센서는 16bit 기준으로 값을 가져오므로
    //8비트씩 2번을 받아야 한다.
    value = TWDR << 8;
    TWCR = _BV(TWINT) | _BV(TWEN);
    while (!(TWCR & _BV(TWINT)));

    value |= TWDR;
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);

    value >>= 8;

    TIMSK = (value >= 33) ? TIMSK | _BV(TOIE2) : TIMSK & ~_BV(TOIE2);

    return value;
}

void TemperatureTask(void* data)
{
    INT8U   value;
    INT8U   err;
    data = data;

    InitI2C();
    write_twi_1byte_nopreset(ATS75_CONFIG_REG, 0x00); // 9비트, Normal
    write_twi_0byte_nopreset(ATS75_TEMP_REG); // Temp Reg 포인팅
    while (1) {
        OS_ENTER_CRITICAL();
        value = ReadTemperature();
        OS_EXIT_CRITICAL();
        // 작성
        OS_ENTER_CRITICAL();
        OSMboxPost(Mbox, (void*)&value);
        OS_EXIT_CRITICAL();
        OSTimeDly(100);
    }
}

void FndTask(void* data)
{
    volatile int i;
    INT8U   err;
    data = data;
    // 작성

    while (1) {
        OS_ENTER_CRITICAL();
        fvalue = *(INT8U*)OSMboxPend(Mbox, 0, &err);
        OS_EXIT_CRITICAL();
        OSTimeDly(100);
    }
}

void FndDisplayTask(void* data)
{
    unsigned char FND_DATA[] = {
       0x3f, // 0
       0x06, // 1
       0x5b, // 2
       0x4f, // 3
       0x66, // 4
       0x6d, // 5
       0x7d, // 6
       0x27, // 7
       0x7f, // 8
       0x6f, // 9
       0x77, // A
       0x7c, // B
       0x39, // C
       0x5e, // D
       0x79, // E
       0x71, // F
       0x80, // .
       0x40, // -
       0x08,  // _
       0x00 // null
    };
    unsigned int num[4];

    data = data;

    DDRC = 0xff;
    DDRG = 0x0f;

    while (1) {
        if ((fvalue & 0x8000) != 0x8000)  // Sign 비트 체크
            num[3] = 19;
        else
        {
            num[3] = 17;
            fvalue = (~fvalue) + 1;   // 2’s Compliment
        }
        num[2] = (fvalue / 10) % 10;
        num[1] = fvalue % 10;
        num[0] = (((fvalue & 0x00ff) & 0x80) == 0x80) * 5;
        while (1)
        {
            for (i = 0; i < 4; i++)
            {
                PORTC = FND_DATA[num[i]];
                PORTG = fnd_sel[i];
                if (i == 1) PORTC |= 0x80;
                OSTimeDlyHMSM(0, 0, 0, 1);
            }
        }
    }
}