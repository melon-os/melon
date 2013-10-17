/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_timer.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013-7-31
    Last Modified :
    Description   : Melon OS timer source file

    History       :
        1.Date      : 2013-7-31
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#include <melon/melon.h>
#include <melon/os_core.h>
#include <melon/os_task.h>
#include <melon/os_timer.h>

#ifdef OS_SUPPORT_TIMER
/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/

/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/

OS_TIMER OS_timer_list[OS_TIMER_CNT_MAX];
OS_TIMER_ID OS_timer_cnt;
OS_MUTEX_ID OS_timer_mutex_id;

/*****************************************************************************
 Prototype    : OS_timer_init
 Input        : void
 Output       : None
 Return Value : OS_RET_CODE
 Calls        :
 Description  : Initialize timer module

*****************************************************************************/
OS_RET_CODE OS_timer_init(void)
{
    OS_ISR_STATE    cpu_sr;
    ULONG32         ulIdx;
    OS_RET_CODE     ret;

    (void)ret; /* neutralize compiler warning */
    OS_ENTER_CRITICAL();
    ret = OS_mutex_create("TimerMutexId", 0, &OS_timer_mutex_id);
    __AFFIRM(OS_RET_SUCCESS == ret);

    for (ulIdx = 0; ulIdx < OS_TIMER_CNT_MAX; ulIdx++)
    {
        OS_timer_list[ulIdx].id = OS_TIMER_ID_INVALID;
        OS_timer_list[ulIdx].original_period = OS_timer_list[ulIdx].period = 0;
        OS_timer_list[ulIdx].state = OS_TIMER_INIT;
        OS_timer_list[ulIdx].times = 0;
        OS_timer_list[ulIdx].callback = NULL;
#ifdef OS_TIMER_SUPPORT_NAME
        OS_timer_list[ulIdx].name[0] = EOC;
#endif
    }
    OS_timer_cnt = 0;
    OS_EXIT_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_timer_create
 Input        : OS_TIMER_NAME name
                OS_TIMER_PERIOD period, in unit ms, the least value is OS_TICK_PERIOD
                OS_TIMER_PERIOD times
                OS_TIMER_CALLBACK callback
 Output       : OS_TIMER_ID *tid
 Return Value : OS_RET_CODE
 Calls        :
 Description  : Create a timer
        The callback function will be called in interrupt context, in other words,
        You cann't call some functions which are forbidden in interrupt context.
        Such as OS_mem_malloc, OS_mutex_lock,etc.
        If pass a 0 to args.times, it means this timer will loop forever until you
        pause or delete it.
*****************************************************************************/
OS_RET_CODE OS_timer_create(OS_TIMER_NAME name, OS_TIMER_PERIOD period,
                                        OS_TIMER_PERIOD times, OS_TIMER_CALLBACK callback,
                                            OS_TIMER_ID *id_ptr)
{
    OS_TIMER_ID ulIdx;
#ifdef OS_TIMER_SUPPORT_NAME
    OS_TIMER_ID ulIdy;
#endif
    if ((NULL == callback) || (NULL == id_ptr) || (0 == period))
    {
        return OS_RET_PARAM;
    }
#ifdef OS_TIMER_SUPPORT_NAME
    if (NULL == name)
    {
        return OS_RET_PARAM;
    }
#endif


    if (period < OS_TICK_PERIOD)
    {
        period = 1;
    }
    else
    {
        period = period / OS_TICK_PERIOD;
    }

    OS_ENTER_TIMER_CRITICAL();
    if (OS_TIMER_CNT_MAX == OS_timer_cnt)
    {
        OS_EXIT_TIMER_CRITICAL();
        return OS_RET_FULL;
    }

    for (ulIdx = 0; ulIdx < OS_TIMER_CNT_MAX; ulIdx++)
    {
        if (OS_TIMER_INIT == OS_timer_list[ulIdx].state)
        {
            OS_timer_list[ulIdx].state = OS_TIMER_STOP;
            break;
        }
    }
    OS_EXIT_TIMER_CRITICAL();
    if (OS_TIMER_CNT_MAX == ulIdx)
    {
        return OS_RET_FULL;
    }

    *id_ptr = ulIdx;
    OS_ENTER_TIMER_CRITICAL();
    OS_timer_list[ulIdx].id = ulIdx;
    OS_timer_list[ulIdx].original_period = OS_timer_list[ulIdx].period = period;
    OS_timer_list[ulIdx].state = OS_TIMER_STOP;
    OS_timer_list[ulIdx].times = times;
    OS_timer_list[ulIdx].callback = callback;

#ifdef OS_TIMER_SUPPORT_NAME
    for (ulIdy = 0; EOC != (OS_timer_list[ulIdx].name[ulIdy] = name[ulIdy]); ulIdy++)
    {
        if (OS_TIMER_NAME_LEN_MAX == ulIdy)
        {
            OS_timer_list[ulIdx].name[OS_TIMER_NAME_LEN_MAX - 1] = EOC;
            break;
        }
    }
#endif
    OS_timer_cnt++;
    OS_EXIT_TIMER_CRITICAL();

    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_timer_delete
 Input        : OS_TIMER_ID tid
 Output       : None
 Return Value : OS_RET_CODE
 Calls        :
 Description  : Delete a timer
*****************************************************************************/
OS_RET_CODE OS_timer_delete(OS_TIMER_ID tid)
{
    if (OS_TIMER_ID_INVALID <= tid)
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_TIMER_CRITICAL();
    OS_timer_list[tid].id = OS_TIMER_ID_INVALID;
    OS_timer_list[tid].original_period = OS_timer_list[tid].period = 0;
    OS_timer_list[tid].state = OS_TIMER_INIT;
    OS_timer_list[tid].times = 0;
    OS_timer_list[tid].callback = NULL;
#ifdef OS_TIMER_SUPPORT_NAME
    OS_timer_list[tid].name[0] = EOC;
#endif
    OS_timer_cnt--;
    OS_EXIT_TIMER_CRITICAL();

    return OS_RET_SUCCESS;
}
/*****************************************************************************
 Prototype    : OS_timer_start
 Input        : OS_TIMER_ID tid
 Output       : None
 Return Value : OS_RET_CODE
 Calls        :
 Description  : Kick a timer to run
*****************************************************************************/
OS_RET_CODE OS_timer_start(OS_TIMER_ID tid)
{
    if (OS_TIMER_ID_INVALID <= tid)
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_TIMER_CRITICAL();

    if (OS_TIMER_STOP != OS_timer_list[tid].state)
    {
        OS_EXIT_TIMER_CRITICAL();
        return OS_RET_BUSY;
    }
    OS_timer_list[tid].state = OS_TIMER_RUN;
    OS_EXIT_TIMER_CRITICAL();

    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_timer_pause
 Input        : OS_TIMER_ID tid
 Output       : None
 Return Value : OS_RET_CODE
 Calls        :
 Description  : Pause a timer
*****************************************************************************/
OS_RET_CODE OS_timer_pause(OS_TIMER_ID tid)
{
    if (OS_TIMER_ID_INVALID <= tid)
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_TIMER_CRITICAL();

    if (OS_TIMER_RUN != OS_timer_list[tid].state)
    {
        OS_EXIT_TIMER_CRITICAL();
        return OS_RET_BUSY;
    }
    OS_timer_list[tid].state = OS_TIMER_PAUSE;
    OS_EXIT_TIMER_CRITICAL();

    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_timer_resume
 Input        : OS_TIMER_ID tid
 Output       : None
 Return Value : OS_RET_CODE
 Calls        :
 Description  : wakeup a timer to run
*****************************************************************************/
OS_RET_CODE OS_timer_resume(OS_TIMER_ID tid)
{
    if (OS_TIMER_ID_INVALID <= tid)
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_TIMER_CRITICAL();
    if (OS_TIMER_PAUSE != OS_timer_list[tid].state)
    {
        OS_EXIT_TIMER_CRITICAL();
        return OS_RET_BUSY;
    }
    OS_timer_list[tid].state = OS_TIMER_RUN;
    OS_EXIT_TIMER_CRITICAL();

    return OS_RET_SUCCESS;
}
#endif /* #ifdef OS_SUPPORT_TIMER */
