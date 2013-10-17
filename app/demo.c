/******************************************************************************
Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

* LICENSING TERMS:
* ---------------
* Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
* If you plan on using Melon in a commercial product you need to contact me to properly license
* its use in your product. I provide ALL the source code for your convenience and to help you experience
* Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
* licensing fee.

File Name     : demo.c
Version       : Initial Draft
Author        : QQ 381624054
Created       : 2013/6/12
Last Modified :
Description   : Melon OS demo entry source file

History       :
1.Date      : 2013/6/12
Author      : QQ 381624054
Modification: Created file
******************************************************************************/
#include <common_func.h>
#include <drv/serial.h>
#include <melon/melon.h>
#include <melon/os_mutex.h>
#include <melon/os_timer.h>
#include <melon/os_mem.h>

/*----------------------------------------------*
* external routine prototypes                  *
*----------------------------------------------*/
extern void Clock_time_demo(void *p_args);
extern void HD_reset(void);
/*----------------------------------------------*
* internal routine prototypes                  *
*----------------------------------------------*/
static void OS_logo(void);
static void melon_demo1(void *p_args);
static void melon_demo2(void *p_args);
static void melon_demo3(void *p_args);
#ifdef OS_SUPPORT_TIMER
static void DemonTimer(void);
#endif
#ifdef OS_SUPPORT_STAT
static void Task_manager(void *p_args);
#endif
#ifdef OS_SUPPORT_MEM
static void Mem_mng_dem(void *param);
#endif
/*----------------------------------------------*
* module-wide global variables                 *
*----------------------------------------------*/
OS_MBOX_ID  mailboxid1 = OS_MBOX_ID_INVALID;
OS_MBOX_ID  mailboxid2 = OS_MBOX_ID_INVALID;

OS_MUTEX_ID mutex_id;
#define DEMO2_NAME  ("Melon_demo2")
#define DEMO3_NAME  ("Melon_demo3")

OS_TASK_ID  demo1_taskid;
OS_TASK_ID  demo2_taskid;
OS_TASK_ID  demo3_taskid;
#ifdef OS_SUPPORT_TIMER
OS_TIMER_ID demo3_demon_timerid;
#endif
#ifndef OS_SUPPORT_MEM
OS_STK  g_TaskMngStk[OS_STKSIZE_DEFAULT/sizeof(OS_STK)];
OS_STK  g_ClkDemoStk[OS_STKSIZE_DEFAULT/sizeof(OS_STK)];
OS_STK  g_SysDemo1Stk[OS_STKSIZE_DEFAULT/sizeof(OS_STK)];
OS_STK  g_SysDemo2Stk[OS_STKSIZE_DEFAULT/sizeof(OS_STK)];
OS_STK  g_SysDemo3Stk[OS_STKSIZE_DEFAULT/sizeof(OS_STK)];
#endif


int melon_entry(void)
{
    OS_TASK_ID  taskid;

    OS_logo();
    OS_init();

/*
 ##################################################
 ####### [START] Add user application code  #######
 ##################################################
*/
    mailboxid1 = OS_MBOX_ID_INVALID;
    mailboxid2 = OS_MBOX_ID_INVALID;
    demo3_taskid = OS_TASK_ID_INVALID;

#ifdef OS_SUPPORT_STAT
#ifdef OS_SUPPORT_MEM
    OS_task_create(Task_manager, NULL, 100, NULL, OS_STKSIZE_DEFAULT,
                        "Task Manager", 0, &taskid);
#else
    OS_task_create(Task_manager, NULL, 100, g_TaskMngStk, OS_GET_STK_SIZE(g_TaskMngStk),
                        "Task Manager", 0, &taskid);

#endif /* #ifdef OS_SUPPORT_MEM */
#endif /* #ifdef OS_SUPPORT_STAT */

#ifdef OS_SUPPORT_MEM
    OS_task_create(Clock_time_demo, NULL, 1, NULL, OS_STKSIZE_DEFAULT,
                        "GUI Calendar", 0, &taskid);
    OS_task_create(melon_demo1, NULL, 123, NULL, OS_STKSIZE_DEFAULT,
                        "Melon_demo1", 0, &demo1_taskid);
    OS_task_create(melon_demo2, NULL, 124, NULL, OS_STKSIZE_DEFAULT,
                        DEMO2_NAME, 0, &demo2_taskid);
#else
    OS_task_create(Clock_time_demo, NULL, 1, g_ClkDemoStk, OS_GET_STK_SIZE(g_ClkDemoStk),
                        "GUI Calendar", 0, &taskid);
    OS_task_create(melon_demo1, NULL, 123, g_SysDemo1Stk, OS_GET_STK_SIZE(g_SysDemo1Stk),
                        "Melon_demo1", 0, &demo1_taskid);
    OS_task_create(melon_demo2, NULL, 124, g_SysDemo2Stk, OS_GET_STK_SIZE(g_SysDemo2Stk),
                        "Melon_demo2", 0, &demo2_taskid);
#endif /* #ifdef OS_SUPPORT_MEM */

#ifdef OS_SUPPORT_TIMER
    OS_timer_create("DemonTimer", 500, 0, DemonTimer, &demo3_demon_timerid);
#endif

#ifdef OS_SUPPORT_MEM
    OS_task_create(Mem_mng_dem, NULL, 126, NULL, OS_STKSIZE_DEFAULT,
        "Mem_mng_dem", 0, &taskid);
#endif /* #ifdef OS_SUPPORT_MEM */
    OS_mutex_create("test mutex", 0, &mutex_id);

/*
 ##################################################
 ####### [END] Add user application code  #########
 ##################################################
*/
    OS_start();
    return 0;
}

void melon_demo1(void *p_args)
{
    OS_MBOX_MSG msg;
    OS_RET_CODE ret;
    (void)p_args;

    OS_mailbox_create(&mailboxid1);
    while (1)
    {
        OS_mutex_lock(mutex_id, 0, 0);
        OS_mutex_unlock(mutex_id);

        ret = OS_mailbox_recv(mailboxid1, &msg, 0);
        if (OS_RET_SUCCESS == ret)
        {
            Serial_printf(VT_GREEN"demo1 rec:0x%x\n\r"VT_DEFAULT, msg.content);
        }
        msg.content = 0;

        OS_task_sleep(1000);
    }
}

void melon_demo2(void *p_args)
{
    OS_MBOX_MSG msg;
    OS_RET_CODE ret;
    (void)p_args;

    OS_mailbox_create(&mailboxid2);
#ifdef OS_SUPPORT_TIMER
    OS_timer_start(demo3_demon_timerid);
#endif
    while (1)
    {
        OS_mutex_trylock(mutex_id);
        OS_mutex_unlock(mutex_id);

        ret = OS_mailbox_recv(mailboxid2, &msg, 100);
        if (OS_RET_SUCCESS == ret)
        {
#ifdef OS_SUPPORT_TIMER
            OS_timer_pause(demo3_demon_timerid);
#endif
            Serial_printf(VT_YELLOW"demo2 rec:0x%x\n\r"VT_DEFAULT, msg.content);
            if ((0x999 == msg.content) && (OS_TASK_UNINIT == OS_task_query_name(DEMO3_NAME)))
            {
                msg.content = 0;
                Serial_printf("create Melon_demo3\r\n");
#ifdef OS_SUPPORT_MEM
                OS_task_create(melon_demo3, NULL, 125, NULL, OS_STKSIZE_DEFAULT,
                    DEMO3_NAME, 0, &demo3_taskid);
#else
                OS_task_create(melon_demo3, NULL, 125, g_SysDemo3Stk, OS_GET_STK_SIZE(g_SysDemo3Stk),
                    "Melon_demo3", 0, &demo3_taskid);
#endif /* #ifdef OS_SUPPORT_MEM */
            }
#ifdef OS_SUPPORT_TIMER
            OS_timer_resume(demo3_demon_timerid);
#endif
        }

        OS_task_sleep(1000);
    }
}
void melon_demo3(void *p_args)
{
    UCHAR8  ucIpt = 0;
    OS_TASK_ID  taskid;
    OS_MBOX_MSG Msg;
#ifndef MANUEL_INPUT
    ULONG32 ulIdx = 0;
    UCHAR8  AutoIpt[] = {
        "w,8,9,9,8,9,8"
        "5,w,w,c"
        "9,9,9,9,9,9,9,9,9,9,9"
        "4,w,8,9,8,b,8,9,5,l,c,u,8,9"
        "e,8,9,4,5,b,c,8,9,9,8"
    };
#endif /* #ifdef RUN_ON_SIMULATOR */
    (void)p_args;

    while (1)
    {
#ifdef MANUEL_INPUT
        ucIpt = sgetc();
#else
        ucIpt = AutoIpt[ulIdx];
        if (++ulIdx == sizeof(AutoIpt))
        {
            ulIdx = 0;
        }
#endif
        switch (ucIpt)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                Serial_printf("del task:%c\r\n", ucIpt);
                OS_task_delete((OS_TASK_ID)(ucIpt - '0'));
                break;
            case 'a':
                Serial_printf("create clock demo\r\n");
#ifdef OS_SUPPORT_MEM
                OS_task_create(Clock_time_demo, NULL, 2, NULL, OS_STKSIZE_DEFAULT,
                    "GUI Calendar", OS_TASK_OPT_UNDELETED, &taskid);
#else
                OS_task_create(Clock_time_demo, NULL, 2, g_ClkDemoStk, OS_GET_STK_SIZE(g_ClkDemoStk),
                                    "GUI Calendar", OS_TASK_OPT_UNDELETED, &taskid);
#endif /* #ifdef OS_SUPPORT_MEM */
                break;
            case 'b':
                Serial_printf("create Melon_demo1\r\n");
#ifdef OS_SUPPORT_MEM
                OS_task_create(melon_demo1, NULL, 123, NULL, OS_STKSIZE_DEFAULT,
                                    "Melon_demo1", 0, &demo1_taskid);
#else
                OS_task_create(melon_demo1, NULL, 123, g_SysDemo1Stk, OS_GET_STK_SIZE(g_SysDemo1Stk),
                                    "Melon_demo1", 0, &demo1_taskid);
#endif /* #ifdef OS_SUPPORT_MEM */
                break;
            case 'c':
                Serial_printf("create Melon_demo2\r\n");
#ifdef OS_SUPPORT_MEM
                OS_task_create(melon_demo2, NULL, 124, NULL, OS_STKSIZE_DEFAULT,
                    DEMO2_NAME, (OS_TASK_OPT)0, &demo2_taskid);
#else
                OS_task_create(melon_demo2, NULL, 124, g_SysDemo2Stk, OS_GET_STK_SIZE(g_SysDemo2Stk),
                    "Melon_demo2", (OS_TASK_OPT)0, &demo2_taskid);
#endif /* #ifdef OS_SUPPORT_MEM */
                break;
            case 'l':
                Serial_printf("lock mutex\r\n");
                OS_mutex_lock(mutex_id, 0, 0);
                Serial_printf("got the mutex\r\n");
                break;
            case 'u':
                Serial_printf("unlock mutex\r\n");
                OS_mutex_unlock(mutex_id);
                break;
            case 'e':
                Serial_printf("Melon_demo3 will exit\r\n");
                OS_task_exit();
                break;
            case '8':
                Msg.dst = demo1_taskid;
                Msg.content = demo1_taskid;
                Serial_printf("send to demo1\r\n");
                OS_mailbox_post(mailboxid1, &Msg, 0);
                break;
            case '9':
                Msg.dst = demo2_taskid;
                Msg.content = demo2_taskid;
                Serial_printf("send to demo2\r\n");
                OS_mailbox_post(mailboxid2, &Msg, 0);
                break;
            case 'w':
                OS_task_sleep(1000);
                break;
            case '`':
                HD_reset();
                break;
            case 'd':
                Serial_set_color(transp);
                break;
            case '-':
                Serial_set_color(underline);
                break;
            case 'r':
                Serial_set_color(red);
                break;
            case 'y':
                Serial_set_color(yellow);
                break;
            case 'g':
                Serial_set_color(green);
                break;
            default:
                break;
        }
        OS_task_sleep(100);
    }
}

#ifdef OS_SUPPORT_TIMER
void DemonTimer(void)
{
    OS_MBOX_MSG msg;
    OS_TASK_STATE   state;

    state = OS_task_query_name(DEMO2_NAME);
    if ((OS_TASK_UNINIT == OS_task_query_name(DEMO3_NAME))
            && ((OS_TASK_READY == state) || (OS_TASK_PEND == state)))
    {
        msg.dst = demo2_taskid;
        msg.content = 0x999;
        OS_mailbox_post(mailboxid2, &msg, 0);
    }
}
#endif /* #ifdef OS_SUPPORT_TIMER */
#ifdef OS_SUPPORT_MEM
void Mem_mng_dem(void *param)
{
    ULONG32 asize[] = {
                        6, 16, 26, 37, 127,
                        764, 513, 32, 511,
                        2048,1024, 5, 8, 9,
                        768, 320, 2048, 4095,
                        4096, 64
                      };
    ULONG32 ulIdx;
    CONST ULONG32 ulTestCnt = GET_ARRAY_CNT(asize);
    ULONG32** pptr;

    while(1)
    {
        OS_lock_scheduler(TRUE);
        pptr = OS_mem_malloc(sizeof(ULONG32*)*ulTestCnt);
        __AFFIRM(NULL != pptr);

        for (ulIdx = 0; ulIdx < ulTestCnt; ulIdx++)
        {
            pptr[ulIdx] = OS_mem_malloc(asize[ulIdx]);
        }
        //OS_mem_info();

        for (ulIdx = 0; ulIdx < ulTestCnt; ulIdx++)
        {
            OS_mem_free(pptr[ulIdx]);
        }
        OS_mem_free(pptr);

        //OS_mem_info();
        OS_lock_scheduler(FALSE);
        OS_task_sleep(5000);
    }
}
#endif /* #ifdef OS_SUPPORT_MEM */
#ifdef OS_SUPPORT_STAT
void Task_manager(void *p_args)
{
    ULONG32 ulIdx;
    OS_TASK_ID    task_id[OS_TASK_CNT_MAX] = {0};
    OS_TASK_TIMER task_usage[OS_TASK_CNT_MAX] = {0};
    OS_STK_SIZE  task_stk_size[OS_TASK_CNT_MAX] = {0};
    OS_STK_SIZE task_curr_size[OS_TASK_CNT_MAX] = {0};
#ifdef OS_SUPPORT_MEM
    OS_TASK_NAME  *task_name;
#else
    OS_TASK_NAME  task_name[OS_TASK_CNT_MAX] = {0};
#endif /* #ifdef OS_SUPPORT_STAT */
    OS_TASK_PRIORITY task_pri[OS_TASK_CNT_MAX] = {0};
    OS_TASK_TIMER task_active_tick[OS_TASK_CNT_MAX] = {0};
    OS_SCHD_PERIOD period, cycle, ctxswtcnt;
    OS_TASK_ID    task_cnt;

    p_args = p_args;

    while (1)
    {
#ifdef OS_SUPPORT_MEM
        task_name = (OS_TASK_NAME*)OS_mem_malloc(sizeof(OS_TASK_NAME)*OS_TASK_CNT_MAX);
        if (NULL == task_name)
        {
            Serial_printf("No mem!!!\r\n");
            goto __wait_next;
        }
#endif /* #ifdef OS_SUPPORT_STAT */
        task_cnt = OS_task_get_stat(task_id, task_usage, task_stk_size, task_curr_size,
        task_name, task_pri, task_active_tick,
        &period, &cycle, &ctxswtcnt);

        Serial_printf("\n\rSched:%04d/%04d--%d\n\r", cycle, period, ctxswtcnt);
        Serial_printf(PRINT_GAP);
        Serial_printf("id   name\t\tpri\ttick\tstack\t useage\n\r");
        Serial_printf(PRINT_GAP);
        for ( ulIdx = 0 ; ulIdx < task_cnt; ulIdx++ )
        {
            Serial_printf("%02d   ", task_id[ulIdx]);
            Serial_printf("%s\t", task_name[ulIdx]);
            Serial_printf("%03d\t", task_pri[ulIdx]);
            Serial_printf("%03d\t", task_active_tick[ulIdx]);
            Serial_printf("%03d/%03d    ", task_curr_size[ulIdx], task_stk_size[ulIdx]);
            Serial_printf("%d%%\n\r", task_usage[ulIdx]);
        }
        Serial_printf(PRINT_GAP);
#ifdef OS_SUPPORT_MEM
        OS_mem_free(task_name);
        task_name = NULL;
__wait_next:
#endif /* #ifdef OS_SUPPORT_STAT */
        OS_task_sleep(1000);
    }
}
#endif /*#ifdef OS_SUPPORT_STAT*/

void OS_logo()
{
    CHAR8 *plogo = OS_LOGO;

    while (1)
    {
        switch (*plogo)
        {
            case '-':
                Serial_set_color(white);
                printf("%c", '-');
                break;
            case '*':
                Serial_set_color(red);
                printf("%c", '*');
                break;
            case '.':
                Serial_set_color(yellow);
                printf("%c", '.');
                break;
            case '\r':
            case '\n':
                printf("%c", *plogo);
                break;
            case EOC:
                Serial_set_color(transp);
#ifdef RUN_ON_BOARD
                Sys_delay_cpu_ms(5000);
#else
                Sys_delay_cpu_ms(100);
#endif
                return;
            default:
                Serial_set_color(green);
                printf("%c", *plogo);
                break;
        }
        plogo++;
    }
}

