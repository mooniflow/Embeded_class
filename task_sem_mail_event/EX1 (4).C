
/*
*********************************************************************************************************
* uC / OS - II
* The Real - Time Kernel
*
*(c)Copyright 1992 - 2002, Jean J.Labrosse, Weston, FL
* All Rights Reserved
*
* EXAMPLE #2
* ********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define          TASK_STK_SIZE     512                /* Size of each task's stacks (# of WORDs)       */
#define N_TASK 4

/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/

OS_STK TaskStartStk[TASK_STK_SIZE];
OS_STK TaskStk[N_TASK][TASK_STK_SIZE];
char TaskData[N_TASK];



OS_EVENT* sem;
OS_EVENT* TxMbox[4];
OS_FLAG_GRP* e_grp;

INT8U majority= 0;
/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void Task(void* data);
static  void  TaskStart(void* data);                  /* Function prototypes of tasks                  */
static  void  TaskStartCreateTasks(void);
static  void  TaskStartDispInit(void);
static  void  TaskStartDisp(void);


/*$PAGE*/
/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main(void)
{
    OS_STK* ptos;
    OS_STK* pbos;
    INT32U  size;

    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);                        /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    OSTaskCreate(TaskStart, (void*)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);

    OSStart();                                             /* Start multitasking                       */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               STARTUP TASK
*********************************************************************************************************
*/

static void  TaskStart(void* pdata)
{
#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    OS_CPU_SR  cpu_sr;
#endif
    INT16S     key;
    INT8U i;
    INT8U err;

    pdata = pdata;                                         /* Prevent compiler warning                 */

    TaskStartDispInit();                                   /* Setup the display                        */

    OS_ENTER_CRITICAL();                                   /* Install uC/OS-II's clock tick ISR        */
    PC_VectSet(0x08, OSTickISR);
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit(); /* Initialize uC/OS-II's statistics         */
    
    sem = OSSemCreate(1); 
    TxMbox[0] = OSMboxCreate((void*)0);
    TxMbox[1] = OSMboxCreate((void*)0);
    TxMbox[2] = OSMboxCreate((void*)0);
    TxMbox[3] = OSMboxCreate((void*)0);
    
    e_grp = OSFlagCreate(0x00, &err);


    TaskStartCreateTasks();                                /* Create all other tasks                   */

    for (;;) {
        TaskStartDisp();                                   /* Update the display                       */

        if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Yes, return to DOS                       */
            }
        }

        OSCtxSwCtr = 0;                                    /* Clear context switch counter             */
        OSTimeDly(OS_TICKS_PER_SEC);                       /* Wait one second                          */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                        INITIALIZE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDispInit(void)
{
    /*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
    /*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
    PC_DispStr(0, 0, "                         uC/OS-II, The Real-Time Kernel                         ", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
    PC_DispStr(0, 1, "                                Jean J. Labrosse                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 2, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 3, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 4, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 5, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 6, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 7, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 8, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 9, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 10, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 11, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 12, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 13, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 14, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 15, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 16, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 17, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 18, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 19, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 20, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 21, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 22, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 23, "                                                                                ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
    PC_DispStr(0, 24, "                            <-PRESS 'ESC' TO QUIT->                             ", DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY + DISP_BLINK);
    /*                                1111111111222222222233333333334444444444555555555566666666667777777777 */
    /*                      01234567890123456789012345678901234567890123456789012345678901234567890123456789 */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                           UPDATE THE DISPLAY
*********************************************************************************************************
*/

static  void  TaskStartDisp(void)
{
    char   s[80];


    sprintf(s, "%5d", OSTaskCtr);                                  /* Display #tasks running               */
    PC_DispStr(18, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%3d", OSCPUUsage);                                 /* Display CPU usage in %               */
    PC_DispStr(36, 22, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "%5d", OSCtxSwCtr);                                 /* Display #context switches per second */
    PC_DispStr(18, 23, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    sprintf(s, "V%4.2f", (float)OSVersion() * 0.01);               /* Display uC/OS-II's version number    */
    PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

    switch (_8087) {                                               /* Display whether FPU present          */
    case 0:
        PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_YELLOW + DISP_BGND_BLUE);
        break;

    case 1:
        PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
        break;

    case 2:
        PC_DispStr(71, 22, "80287 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
        break;

    case 3:
        PC_DispStr(71, 22, "80387 FPU", DISP_FGND_YELLOW + DISP_BGND_BLUE);
        break;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             CREATE TASKS
*********************************************************************************************************
*/

static  void  TaskStartCreateTasks(void)
{
    INT8U i;

    for (i = 0; i < N_TASK; i++) {
        TaskData[i] = '0' + i;
        OSTaskCreate(Task, (void*)&TaskData[i], &TaskStk[i][TASK_STK_SIZE - 1], i + 1);
    }
}

void Task(void* pdata) {
    INT8U err;
    INT8U randn;

    INT8U i, j;

    char push_letter[3];
    char get_letter[3];
  
    int task_number = (int)(*(char*)pdata - 48);//각 task의 index이다. pdata는 char타입이기 때문에 ascii 기준 -48을 하면 int형으로 바뀐다.


    int fgnd_color, bgnd_color;
    char s[10];

   


    if (*(char*)pdata == '3') {
        for (;;) {

          
            OSFlagPend(e_grp, 0x0F, OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME, 0, &err);
            OSSemPend(sem, 0, &err);
            if (majority >= 2) {
                for (j = 5; j < 24; j++) {
                    for (i = 0; i < 80; i++) {
                        PC_DispChar(i, j, ' ', DISP_BGND_BLUE + DISP_FGND_BLUE);
                    }
                }
            }
            else {
                for (j = 5; j < 24; j++) {
                    for (i = 0; i < 80; i++) {
                        PC_DispChar(i, j, ' ', DISP_BGND_RED + DISP_FGND_RED);
                    }
                }
            }
            OSSemPost(sem);
            OSTimeDlyHMSM(0, 0, 5, 0);

        }
    }

    else {
        for (;;) {
            OSSemPend(sem, 0, &err);
            randn = random(2);
            OSSemPost(sem);



            if (*(char*)pdata == '0') { 
                if (randn == 0) {
                    push_letter[0] = 'O';
                }
                else {
                    push_letter[0] = 'X';
                }
                OSMboxPost(TxMbox[task_number], (void*)&push_letter[0]);
                PC_DispChar(9 + 18 * task_number, 4, push_letter[0], DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
                
                get_letter[0] = *(char*)OSMboxPend(TxMbox[0], 0, &err);
                get_letter[1] = *(char*)OSMboxPend(TxMbox[1], 0, &err);
                get_letter[2] = *(char*)OSMboxPend(TxMbox[2], 0, &err);

                OSSemPend(sem, 0, &err);
                majority = 0;
                for (i = 0; i < 3; i++) {
                    if (get_letter[i] == 'O') {
                        majority++;
                    }
                } 
                OSSemPost(sem);
                OSFlagPost(e_grp, 0x0F, OS_FLAG_SET, &err);
                
            }

            else if (*(char*)pdata == '1') {
                if (randn == 0) {
                    push_letter[1] = 'O';
                }
                else {
                    push_letter[1] = 'X';
                }
                OSMboxPost(TxMbox[task_number], (void*)&push_letter[1]); 
                PC_DispChar(9 + 18 * task_number, 4, push_letter[1], DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);


                
            }

            else if (*(char*)pdata == '2') {
                if (randn == 0) {
                    push_letter[2] = 'O';
                }
                else {
                    push_letter[2] = 'X';
                }
                OSMboxPost(TxMbox[task_number], (void*)&push_letter[2]);
                PC_DispChar(9 + 18 * task_number, 4, push_letter[2], DISP_FGND_BLACK + DISP_BGND_LIGHT_GRAY);
               
            }
            OSTimeDlyHMSM(0, 0, 5, 0);
        }
    }
}