/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_core.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS core source file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#include <melon/melon.h>
#include <melon/os_core.h>
#include <melon/os_task.h>
#include <melon/os_timer.h>
#include <melon/os_mem.h>

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/
/* TCB list */
extern OS_TCB  OS_TCBList[OS_TASK_CNT_MAX];
OS_TCB *OS_pCurrTask;
OS_TCB *OS_pNextTask;
#ifdef OS_SUPPORT_TIMER
extern OS_TIMER OS_timer_list[OS_TIMER_CNT_MAX];
extern OS_TIMER_ID  OS_timer_cnt;
#endif
/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/
/* This struct holds OS information */
OS_INFO OS_info;
/* Stack for interrupt handlers */
OS_STK OS_handler_main_stk[256];
/* Pointer to OS_handler_main_stk */
OS_STK *OS_pHandler_main_stk;

/* Stack of the famous idle task */
OS_STK OS_idle_task_stk[32];
#ifdef OS_SUPPORT_STAT
/* Stack of stat task */
OS_STK OS_stat_task_stk[64];
#endif /*#ifdef OS_SUPPORT_STAT*/

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

static OS_TASK_ID __next_task(void);

/*****************************************************************************
 Prototype    : OS_Init
 Input        : void
 Output       : None
 Return Value : void
 Description  : OS init function
 Calls        :
        Call this function before all the other functions of melon.
*****************************************************************************/
void OS_init(void)
{
    OS_ISR_STATE    cpu_sr;
    OS_TASK_ID      idle_task_id;
#ifdef OS_SUPPORT_STAT
    OS_TASK_ID  stat_task_id;
#endif /*#ifdef OS_SUPPORT_STAT*/

    OS_ENTER_CRITICAL();
    OS_info.state = OS_STATE_INIT;
    OS_info.task_cnt = 0;
    OS_info.sched_period = 0;
    OS_info.ctx_swt_cnt = 0;
    OS_pCurrTask = &OS_TCBList[OS_IDLE_TASK_ID];
    OS_pNextTask = OS_pCurrTask;
    OS_pHandler_main_stk = OS_GET_STK_TOP(OS_handler_main_stk);
    OS_EXIT_CRITICAL();

    OS_task_init();
    OS_mutex_init();
    OS_mailbox_init();
#ifdef OS_SUPPORT_TIMER
    OS_timer_init();
#endif
#ifdef OS_SUPPORT_MEM
    OS_mem_init();
#endif /* #ifdef OS_SUPPORT_MEM */
    OS_tick_init(OS_TICK_HZ);

    /*create idle task*/
    OS_task_create(OS_task_idle, NULL, OS_TASK_PRIORITY_LOWEST,
        OS_idle_task_stk, OS_GET_STK_SIZE(OS_idle_task_stk),
            "OS Idle task",
                OS_TASK_OPT_SYSTEM | OS_TASK_OPT_UNDELETED | OS_TASK_OPT_STATIC,
                    &idle_task_id);
#ifdef OS_SUPPORT_STAT
    /* create stat task */
    OS_task_create(OS_task_stat, NULL, 0,
        OS_stat_task_stk, OS_GET_STK_SIZE(OS_stat_task_stk),
            "OS Stat task",
                OS_TASK_OPT_SYSTEM | OS_TASK_OPT_UNDELETED | OS_TASK_OPT_STATIC,
                    &stat_task_id);
#endif /*#ifdef OS_SUPPORT_STAT*/

    return;
}

/*****************************************************************************
 Prototype    : OS_start
 Input        : void
 Output       : None
 Return Value : void
 Description  : melon starts to run
 Calls        :
*****************************************************************************/
void OS_start(void)
{
    if (OS_info.task_cnt < OS_TASK_CNT_MIN)
    {
        return;
    }

    /*
        Init main stack after OS_pHandler_main_stk got value,
        and I must call it in a unreturn function,
        because mainstk will be  modified in OS_mainstk_init, LR is missed,
        so it can not return normally
    */
    OS_mainstk_init();
    OS_tick_start();
    OS_info.state = OS_STATE_RUN;
    OS_schedule(); /* Certainly a while(1) can be here, but I prefer this way */
}

/*****************************************************************************
 Prototype    : OS_get_state
 Input        : void
 Output       : None
 Return Value : OS_STATE
 Description  : Get OS state
 Calls        :
        Can not be called in interrupt context
*****************************************************************************/
OS_STATE OS_get_state(void)
{
    OS_ISR_STATE    cpu_sr;
    OS_STATE        state;

    OS_ENTER_CRITICAL();
    state = OS_info.state;
    OS_EXIT_CRITICAL();
    return state;
}

/*****************************************************************************
 Prototype    : OS_pause
 Input        : void
 Output       : None
 Return Value : void
 Description  : Pause OS that means make it to be locked from schedule
 Calls        :
*****************************************************************************/
void OS_lock_scheduler(BOOL b)
{
    OS_ISR_STATE cpu_sr;

    OS_ENTER_CRITICAL();
    if (b)
    {
        OS_tick_stop();
        OS_info.state = OS_STATE_SCHED_LOCK;
    }
    else
    {
        OS_tick_start();
        OS_info.state = OS_STATE_RUN;
    }
    OS_EXIT_CRITICAL();

    return;
}

/*****************************************************************************
 Prototype    : OS_schedule
 Input        : void
 Output       : None
 Return Value : void
 Description  : task schedule function
 Calls        :
        Only OS level code can call this function
*****************************************************************************/
void OS_schedule(void)
{
    OS_ISR_STATE    cpu_sr;
    OS_TASK_ID      next_task;

    OS_ENTER_CRITICAL();

    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return;
    }

    next_task = __next_task();
    if (0 != OS_TCBList[next_task].active_tick)
    {
        OS_TCBList[next_task].active_tick--;
    }

    if (next_task == OS_pCurrTask->id)
    {
        OS_EXIT_CRITICAL();
        return;
    }

#ifdef OS_SWX_TICK_REFRESH
    OS_pCurrTask->switch_counter--;
    OS_TCBList[next_task].switch_counter++;

#endif /* #ifdef OS_SWX_TICK_REFRESH */
    /*
        No need to set the OS_pCurrTask state flag,
        the caller should already set this flag
    */    
    OS_pNextTask = &OS_TCBList[next_task];
    OS_pNextTask->state = OS_TASK_RUN;
    /* copy OS_pNextTask to OS_pCurrTask in OS_ctx_switch_t() */
    OS_EXIT_CRITICAL();
    OS_ctx_switch_t();
    return;
}

/*****************************************************************************
 Prototype    : OS_tick_schedule
 Input        : void
 Output       : None
 Return Value : void
 Description  : OS tick interrupt handler
 Calls        :
        Only OS level code can call this function
*****************************************************************************/
void OS_tick_schedule(void) /* interruptr handler */
{
    ULONG32 ulIdx = 0;
    OS_TASK_ID      valid_cnt = 0;
    OS_ISR_STATE    cpu_sr;
    OS_TASK_ID      next_task;

    /* decrease timer */
    OS_ENTER_CRITICAL();
    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return;
    }
    OS_info.ctx_swt_cnt++;

#ifdef OS_SUPPORT_TIMER
    for (ulIdx = 0, valid_cnt = 0; valid_cnt < OS_timer_cnt; ulIdx++)
    {
        switch (OS_timer_list[ulIdx].state)
        {
            case OS_TIMER_INIT:
                continue;
            case OS_TIMER_STOP:
            case OS_TIMER_PAUSE:
                valid_cnt++;
                continue;
            case OS_TIMER_RUN:
                valid_cnt++;
                switch (OS_timer_list[ulIdx].times)
                {
                    case 0:
                        if (0 != OS_timer_list[ulIdx].period)
                        {
                            OS_timer_list[ulIdx].period--;
                        }
                        else
                        {
                            OS_timer_list[ulIdx].period = OS_timer_list[ulIdx].original_period;
                            OS_timer_list[ulIdx].callback();
                        }
                        break;
                    case 1:
                        OS_timer_list[ulIdx].state = OS_TIMER_STOP;
                        OS_timer_list[ulIdx].callback();
                        break;
                    default:
                        if (0 != OS_timer_list[ulIdx].period)
                        {
                            OS_timer_list[ulIdx].period--;
                        }
                        else
                        {
                            OS_timer_list[ulIdx].times--;
                            OS_timer_list[ulIdx].period = OS_timer_list[ulIdx].original_period;
                            OS_timer_list[ulIdx].callback();
                        }
                        break;
                }
                continue;
        }
    }
#endif

    /* decrease sleep_tick and wake up these tasks whose sleep_tick is 0 */
    for (ulIdx = 0, valid_cnt = 0; valid_cnt < OS_info.task_cnt; ulIdx++)
    {
        switch (OS_TCBList[ulIdx].state)
        {
            case OS_TASK_PEND:
                if (OS_TASK_PEND_TIMER == OS_TCBList[ulIdx].pend.timer.type)
                {
                   if (0 < OS_TCBList[ulIdx].pend.timer.tick)
                   {
                       OS_TCBList[ulIdx].pend.timer.tick--;
                       if (0 == OS_TCBList[ulIdx].pend.timer.tick)
                       {
                           OS_TCBList[ulIdx].state = OS_TASK_READY;
                       }
                   }
                }
                else if (OS_TASK_PEND_MUTEX == OS_TCBList[ulIdx].pend.mutex.type)
                {
                    switch (OS_TCBList[ulIdx].pend.mutex.tick)
                    {
                        case 0: /* continue to wait */
                            break;
                        case 1: /* pend_tick runs out, timeout */
                            OS_TCBList[ulIdx].pend.mutex.ret = OS_MUTEX_PEND_TIMEOUT;
                            OS_TCBList[ulIdx].state = OS_TASK_READY; /* wake up this task */
                            break;
                        default: /* decrease pend tick */
                            OS_TCBList[ulIdx].pend.mutex.tick--;
                            break;
                    }
                }
                else if (OS_TASK_PEND_MBOX == OS_TCBList[ulIdx].pend.mbox.type)
                {
                    switch (OS_TCBList[ulIdx].pend.mbox.tick)
                    {
                        case 0: /* continue to wait */
                            break;
                        case 1: /* pend_tick runs out, timeout */
                            OS_TCBList[ulIdx].pend.mbox.ret = OS_MBOX_PEND_TIMEOUT;
                            OS_TCBList[ulIdx].state = OS_TASK_READY; /* wake up this task */
                            break;
                        default: /* decrease pend tick */
                            OS_TCBList[ulIdx].pend.mbox.tick--;
                            break;
                    }
                }
                valid_cnt++;
                break;
            case OS_TASK_READY:
            case OS_TASK_RUN:
                valid_cnt++;
                break;
            case OS_TASK_UNINIT:
            case OS_TASK_INIT:
                /* skip the task which has already been deleted */
                break;
        }
    }

    /* if OS_info.sched_cycle decrease to 0, refresh all tasks's active_tick */
    if (OS_info.sched_counter == OS_info.sched_period)
    {
        OS_info.sched_counter = 0;
        for (ulIdx = 0, valid_cnt = 0; valid_cnt < OS_info.task_cnt; ulIdx++)
        {
            /* skip the deleted and init task */
            if ((OS_TASK_UNINIT != OS_TCBList[ulIdx].state)
                && (OS_TASK_INIT != OS_TCBList[ulIdx].state))
            {
                OS_TCBList[ulIdx].active_tick = OS_TASK_WEIGHT(OS_TCBList[ulIdx].priority);
                valid_cnt++;
            }
        }
    }
    else
    {
        OS_info.sched_counter++;
    }

    next_task = __next_task();
    if (0 != OS_TCBList[next_task].active_tick)
    {
        OS_TCBList[next_task].active_tick--;
    }
#ifdef OS_SUPPORT_STAT
    OS_TCBList[next_task].switch_counter++;
#endif /*#ifdef OS_SUPPORT_STAT*/

    if (next_task == OS_pCurrTask->id)
    {
        OS_EXIT_CRITICAL();
        return;
    }

    /* in case of when a task is doing exit */
    if (OS_TASK_RUN == OS_pCurrTask->state)
    {
        OS_pCurrTask->state = OS_TASK_READY;
    }

    OS_pNextTask = &OS_TCBList[next_task];
    OS_pNextTask->state = OS_TASK_RUN;
    /* copy OS_pNextTask to OS_pCurrTask in OS_ctx_switch_i() */
    OS_EXIT_CRITICAL();
    OS_ctx_switch_i();
    return;
}

/*****************************************************************************
 Prototype    : __next_task
 Input        : void
 Output       : None
 Return Value : OS_TASK_ID
 Description  : return the next task which is ready to run or already be
                running
 Calls        :
        Only OS level code can call this function
*****************************************************************************/
OS_TASK_ID __next_task(void)
{
    ULONG32 ulIdx = 0;
    OS_TASK_ID  valid_cnt = 0;
    ULONG32 candt_task_id = OS_IDLE_TASK_ID;
    ULONG32 candt_task_id_0tick = OS_IDLE_TASK_ID;
    OS_TASK_PRIORITY candt_highest_pri = OS_TASK_PRIORITY_LOWEST;
    OS_TASK_PRIORITY candt_highest_pri_0tick = OS_TASK_PRIORITY_LOWEST;

    /* find next ready task which has the biggest active_tick */
    for (ulIdx = 0, valid_cnt = 0 ; valid_cnt < OS_info.task_cnt; ulIdx++ )
    {
        switch (OS_TCBList[ulIdx].state)
        {
            case OS_TASK_PEND:
                valid_cnt++;
                break;
            case OS_TASK_READY:
            case OS_TASK_RUN:
                /*
                    found some task has active_tick bigger than 0,
                    then choose the next task from these task.
                */
                if (OS_TCBList[ulIdx].active_tick > 0)
                {
                    if (OS_TCBList[ulIdx].priority < candt_highest_pri)
                    {
                        candt_highest_pri = OS_TCBList[ulIdx].priority;
                        candt_task_id = ulIdx;
                    }
                }
                else
                {
                    /* find the highest pri task in these task with 0 active_tick */
                    if (OS_TCBList[ulIdx].priority < candt_highest_pri_0tick)
                    {
                        candt_highest_pri_0tick = OS_TCBList[ulIdx].priority;
                        candt_task_id_0tick = ulIdx;
                    }
                }
                valid_cnt++;
                break;
            case OS_TASK_UNINIT:
            case OS_TASK_INIT:
                /* skip the task which has already been deleted */
                break;
        }
    }

    if (OS_IDLE_TASK_ID == candt_task_id)
    {
        candt_task_id = candt_task_id_0tick;
    }

    return candt_task_id;
}

/*****************************************************************************
 Prototype    : __strcpy
 Input        : CHAR8 *dst
                CHAR8 *src
 Output       : None
 Return Value : CHAR8*
 Description  : copy string from src to dst
 Calls        :
        Only OS level code can call this function
*****************************************************************************/
CHAR8*  __strcpy(CHAR8 *dst, CHAR8 *src)
{
    __AFFIRM((NULL != dst) && (NULL != src));
    while (EOC != (*dst++ = *src++));
    return dst;
}

/*****************************************************************************
 Prototype    : __strcmp
 Input        : CHAR8 *dst
                CHAR8 *src
 Output       : None
 Return Value : CHAR8*
 Description  : compare string between src and dst
 Calls        :
        Only OS level code can call this function
*****************************************************************************/
INT32  __strcmp(CHAR8 *dst, CHAR8 *src)
{
    __AFFIRM((NULL != dst) && (NULL != src));
    while ((EOC != *dst) && (*dst == *src))
    {
      dst++;
      src++;
    }

    return (INT32)(*dst - *src);
}
