#include "includes.h"

#define F_CPU	16000000UL	// CPU frequency = 16 Mhz
#include <avr/io.h>	
#include <util/delay.h>

#define  TASK_STK_SIZE  OS_TASK_DEF_STK_SIZE            

OS_STK          LedTaskStk[TASK_STK_SIZE];


void  LedTask(void *data);


int main (void)
{
  OSInit();

  OS_ENTER_CRITICAL(); // set timer
  TCCR0=0x07;  // 0000 0111  0 timer prescaler 1024
  TIMSK=_BV(TOIE0); // set 0 timer overflow interrupt                   
  TCNT0=256-(CPU_CLOCK_HZ/OS_TICKS_PER_SEC/ 1024);   
  OS_EXIT_CRITICAL();
  
  OSTaskCreate(LedTask, (void *)0, (void *)&LedTaskStk[TASK_STK_SIZE - 1], 0);

  OSStart();                         
  
  return 0;
}


void LedTask (void *data)
{
  data = data;    
  // LED Task
  DDRA = 0xff;

  for (;;) {
	  PORTA = 0xff;
  }
}
