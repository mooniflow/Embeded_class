#include "includes.h"

#define F_CPU	16000000UL	// CPU frequency = 16 Mhz
#include <avr/io.h>
#include <util/delay.h>

#define UCHAR unsigned char // UCHAR ����
#define USHORT unsigned short // USHORT ����
#define ATS75_CONFIG_REG 1
#define ATS75_TEMP_REG 0
#define ATS75_ADDR 0x98 // 0b10011000, 7��Ʈ�� 1��Ʈ left shift

#define  TASK_STK_SIZE  OS_TASK_DEF_STK_SIZE
#define  N_TASKS        3

OS_STK       TaskStk[N_TASKS][TASK_STK_SIZE];

// 1. ���� �ڽ� ���� ���� �ۼ��ϼ���. 


void write_twi_1byte_nopreset(UCHAR reg, UCHAR data);
void write_twi_0byte_nopreset(UCHAR reg);
void  TemperatureTask(void *data);
void  FndTask(void *data);
void  FndDisplayTask(void *data);

INT8U	err;
 
int main (void)
{
  OSInit();
  OS_ENTER_CRITICAL();
  TCCR0 = 0x07;
  TIMSK = _BV(TOIE0);
  TCNT0 = 256 - (CPU_CLOCK_HZ / OS_TICKS_PER_SEC / 1024);
  OS_EXIT_CRITICAL();

  Mbox = OSMboxCreate((void*)0);
  MsgQueue = OSQCreate(&MsgQueueTbl[0], 3);

  OSTaskCreate(TemperatureTask, (void *)0, (void *)&TaskStk[0][TASK_STK_SIZE - 1], 0);
  OSTaskCreate(FndTask, (void *)0, (void *)&TaskStk[1][TASK_STK_SIZE - 1], 1);
  OSTaskCreate(FndDisplayTask, (void *)0, (void *)&TaskStk[2][TASK_STK_SIZE - 1], 2);
  OSStart();

  return 0;
}

void InitI2C()
{
    PORTD = 3; 						// For Pull-up override value
    SFIOR &= ~(1 << PUD); 			// PUD
    TWSR = 0; 						// TWPS0 = 0, TWPS1 = 0
    TWBR = 32;						// for 100  K Hz bus clock
	TWCR = _BV(TWEA) | _BV(TWEN);	// TWEA = Ack pulse is generated
									// TWEN = TWI ������ �����ϰ� �Ѵ�
}

int ReadTemperature(void)
{
	int value;

	TWCR = _BV(TWSTA) | _BV(TWINT) | _BV(TWEN);
	while(!(TWCR & _BV(TWINT)));

	TWDR = 0x98 + 1; //TEMP_I2C_ADDR + 1
	TWCR = _BV(TWINT) | _BV(TWEN);
	while(!(TWCR & _BV(TWINT)));

	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
	while(!(TWCR & _BV(TWINT)));

	//�µ������� 16bit �������� ���� �������Ƿ�
	//8��Ʈ�� 2���� �޾ƾ� �Ѵ�.
	value = TWDR; 
	TWCR = _BV(TWINT) | _BV(TWEN);
	while(!(TWCR & _BV(TWINT)));

	value = ((value<< 8)|TWDR);
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);

	//value >>= 8;
	TIMSK = (value >= 33) ? TIMSK | _BV(TOIE2): TIMSK & ~_BV(TOIE2);

	return value;
}

void write_twi_1byte_nopreset(UCHAR reg, UCHAR data)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // START ����
	while (((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10)); // ACK�� ��ٸ�
	TWDR = ATS75_ADDR | 0;  // SLA+W �غ�, W=0
	TWCR = (1 << TWINT) | (1 << TWEN);  // SLA+W ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18);
	TWDR = reg;    // aTS75 Reg �� �غ�
	TWCR = (1 << TWINT) | (1 << TWEN);  // aTS75 Reg �� ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28);
	TWDR = data;    // DATA �غ�
	TWCR = (1 << TWINT) | (1 << TWEN);  // DATA ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28);
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // STOP ����
}
void write_twi_0byte_nopreset(UCHAR reg)
{
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN); // START ����
	while (((TWCR & (1 << TWINT)) == 0x00) || ((TWSR & 0xf8) != 0x08 && (TWSR & 0xf8) != 0x10));  // ACK�� ��ٸ�
	TWDR = ATS75_ADDR | 0; // SLA+W �غ�, W=0
	TWCR = (1 << TWINT) | (1 << TWEN);  // SLA+W ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xf8) != 0x18);
	TWDR = reg;    // aTS75 Reg �� �غ�
	TWCR = (1 << TWINT) | (1 << TWEN);  // aTS75 Reg �� ����
	while (((TWCR & (1 << TWINT)) == 0x00) || (TWSR & 0xF8) != 0x28);
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); // STOP ����
}


void TemperatureTask (void *data)
{
  data = data;
  int	value;
  InitI2C();

  write_twi_1byte_nopreset(ATS75_CONFIG_REG, 0x00); // 9��Ʈ, Normal
  write_twi_0byte_nopreset(ATS75_TEMP_REG);
  while (1)  {
	  value = ReadTemperature();
	  // 2. Mailbox �ڵ� �ۼ��ϼ���.  

	  OSTimeDlyHMSM(0, 0, 1, 0);
  }
}

void FndTask (void *data)
{
	int value;
	data = data;
    
	while (1) {
	    // 3. Mailbox, �µ� ������ ���� �ڵ� �ۼ��Ͻÿ�. 
		
		OSTimeDlyHMSM(0, 0, 1, 0);
	}
}


void FndDisplayTask (void *data)
{ 
    UCHAR FND_DATA[ ]= {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x40, 0x00 };
    UCHAR fnd_sel[4] = {0x01, 0x02, 0x04, 0x08};
	UCHAR value_int, num[4];
	INT8U	err;
    int i;
	data = data;
    DDRC = 0xff;
    DDRG = 0x0f;
	
	// 4. ȭ�� ��� �ڵ� �ۼ��Ͻÿ�. 

    while(1)  
    {
      for(i=0; i<4; i++)
      {
       
        OSTimeDlyHMSM(0, 0, 0, 1);
      } 
     }  

}


