/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_task.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS task source file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#include <melon/melon.h>
#include <melon/os_core.h>
#include <melon/os_task.h>
#include <melon/os_mem.h>
/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/
extern OS_INFO OS_info;
extern OS_TCB *OS_pCurrTask;
extern OS_TCB *OS_pNextTask;
/* TCB list, holds all task's control information  */
OS_TCB  OS_TCBList[OS_TASK_CNT_MAX];

#ifdef OS_SUPPORT_STAT
/* Vars below hold all tasks's information copies for stat task */
OS_TASK_ID    OS_task_id[OS_TASK_CNT_MAX] = {0};
OS_TASK_TIMER OS_task_usage[OS_TASK_CNT_MAX] = {0};
OS_STK_SIZE OS_task_stk_currsize[OS_TASK_CNT_MAX] = {0};
OS_TASK_NAME  OS_task_name[OS_TASK_CNT_MAX] = {0};
OS_TASK_PRIORITY OS_task_pri[OS_TASK_CNT_MAX] = {0};
OS_TASK_TIMER OS_task_active_tick[OS_TASK_CNT_MAX] = {0};
OS_SCHD_PERIOD OS_sched_period = 0;
OS_SCHD_PERIOD OS_sched_cycle = 0;
OS_SCHD_PERIOD OS_ctx_swt_tick = 0;
OS_TASK_ID OS_task_cnt = 0;
#endif /*#ifdef OS_SUPPORT_STAT*/

/*****************************************************************************
 Prototype    : OS_task_init
 Input        : void
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Init task block
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_task_init(void)
{
    OS_ISR_STATE cpu_sr;
    ULONG32 ulIdx = 0;

    OS_ENTER_CRITICAL();
    for ( ulIdx = 0; ulIdx < OS_TASK_CNT_MAX; ulIdx++ )
    {
        OS_TCBList[ulIdx].id = 0;
        OS_TCBList[ulIdx].state = OS_TASK_UNINIT;
        OS_TCBList[ulIdx].pend.whole = 0;
        OS_TCBList[ulIdx].priority = 0;
        OS_TCBList[ulIdx].original_priority = 0;
        OS_TCBList[ulIdx].stk_tos = NULL;
        OS_TCBList[ulIdx].name[0] = EOC;
    }

    OS_EXIT_CRITICAL();

    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_task_create
 Input        : void (*task)(void *pParam), pointer to task code
                void *pParam,               pointer to a user supplied data area that will be
                                            passed to the task when the task first executes
                OS_STK *pstk_top,           pointer to the top of stack
                OS_TASK_PRIORITY priority   task priority
                OS_TASK_NAME task_name      task name, a 16char array
                OS_TASK_ID *id              holds task id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : create a new task
        The bigger priority value has the lower priority,
        If you would like to create a task including division operation,
        You should add OS_TASK_OPT_DIV to the opt param.
        You can give the stack memory manually by pass a valid pointer to pstk,
        Or you can set NULL to pstk and give size of stack, kernel will
        allocate one memory partition automaticly.
 Calls        :
        Task context is freely to call this function.
        Interrupt context is forbidden to call it.
*****************************************************************************/
OS_RET_CODE OS_task_create(void (*task)(void *pParam), void *pParam,
                                      OS_TASK_PRIORITY priority,
                                      OS_STK *pstk, OS_STK_SIZE size,
                                       OS_TASK_NAME name, OS_TASK_OPT opt,
                                        OS_TASK_ID *id)
{
    OS_ISR_STATE    cpu_sr;
    OS_TASK_ID      task_id;
    ULONG32         ulIdx;
    OS_STK          *pstk_top;

    if ((NULL == task) || (NULL == name) || (NULL == id))
    {
        return OS_RET_PARAM;
    }

    if (OS_TASK_PRIORITY_LOWEST < priority)
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_CRITICAL();
    for (ulIdx = 0 ; ulIdx < OS_TASK_CNT_MAX; ulIdx++)
    {
        if (OS_TASK_UNINIT == OS_TCBList[ulIdx].state)
        {
            OS_TCBList[ulIdx].state = OS_TASK_INIT;
            break;
        }
    }

    if (OS_TASK_CNT_MAX == ulIdx)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NO_MEM;
    }

    *id = task_id = ulIdx;
    OS_EXIT_CRITICAL();

    if (NULL == pstk) /* malloc from system heap */
    {
#ifdef OS_SUPPORT_MEM
        size = (0 == size) ? OS_STKSIZE_DEFAULT : size;
        pstk = OS_mem_malloc_stk(size, task_id);
        if (NULL == pstk)
        {
            OS_ENTER_CRITICAL();
            OS_TCBList[task_id].state = OS_TASK_UNINIT;
            OS_EXIT_CRITICAL();
            return OS_RET_NO_MEM;
        }
        opt |= OS_TASK_OPT_MALLOC;
#else
        OS_EXIT_CRITICAL();
        return OS_RET_PARAM;
#endif /* #ifdef OS_SUPPORT_MEM */
    }
    else
    {
        if (0 == size)
        {
            OS_ENTER_CRITICAL();
            OS_TCBList[task_id].state = OS_TASK_UNINIT;
            OS_EXIT_CRITICAL();
            return OS_RET_PARAM;
        }
        opt |= OS_TASK_OPT_STATIC;
    }

    pstk_top = pstk + (size/(sizeof(OS_STK))) - 1;
    /* Initialize the task's stack */
    pstk_top = OS_task_stack_init(task, pParam, pstk_top, 0);

    OS_ENTER_CRITICAL();
    OS_TCBList[task_id].pend.whole = 0;
    OS_TCBList[task_id].id = task_id;
    OS_TCBList[task_id].priority = priority;
    OS_TCBList[task_id].original_priority = priority;
    OS_TCBList[task_id].stk_tos = pstk_top;
    OS_TCBList[task_id].stk_bos = pstk;
    OS_TCBList[task_id].stk_size = size;
    OS_TCBList[task_id].active_tick = OS_TASK_WEIGHT(priority);

#ifdef OS_SUPPORT_STAT
    OS_TCBList[task_id].switch_counter = 0;
#endif /*#ifdef OS_SUPPORT_STAT*/
    OS_TCBList[task_id].opt = opt;

    ulIdx = 0;
    while (EOC != (OS_TCBList[task_id].name[ulIdx] = name[ulIdx]))
    {
        if (OS_TASK_NAME_LEN_MAX == ++ulIdx)
        {
            OS_TCBList[task_id].name[OS_TASK_NAME_LEN_MAX - 1] = EOC;
            break;
        }
    }

    OS_TCBList[task_id].state = OS_TASK_READY;

    /* add os schedue period */
    OS_info.sched_period += OS_TCBList[task_id].active_tick;
    OS_info.sched_counter = 0;
    OS_info.task_cnt++;

    if (OS_STATE_RUN == OS_info.state)
    {
        OS_EXIT_CRITICAL();
        OS_schedule();
    }

    OS_EXIT_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_task_query_id
 Input        : OS_TASK_ID task_id
 Output       : None
 Return Value : OS_TASK_STATE
 Description  : Query task state by id
 Calls        :
*****************************************************************************/
OS_TASK_STATE OS_task_query_id(OS_TASK_ID task_id)
{
    OS_ISR_STATE    cpu_sr;
    OS_TASK_STATE   state;
    if (task_id >= OS_TASK_ID_INVALID)
    {
        return OS_TASK_UNINIT;
    }

    OS_ENTER_CRITICAL()
    state = OS_TCBList[task_id].state;
    OS_EXIT_CRITICAL();

    return state;
}

/*****************************************************************************
 Prototype    : OS_task_query_name
 Input        : CHAR8 *pname
 Output       : None
 Return Value : OS_TASK_STATE
 Description  : Query task state by name
 Calls        :

*****************************************************************************/
OS_TASK_STATE OS_task_query_name(CHAR8 *pname)
{
    OS_ISR_STATE    cpu_sr;
    OS_TASK_STATE   state;
    OS_TASK_ID      ulIdx;

    if (NULL == pname)
    {
        return OS_TASK_UNINIT;
    }

    OS_ENTER_CRITICAL();
    for (ulIdx = 0; ulIdx < OS_TASK_CNT_MAX; ulIdx++)
    {
        if (OS_TASK_UNINIT != OS_TCBList[ulIdx].state)
        {
            if (0 == __strcmp(OS_TCBList[ulIdx].name, pname))
            {
                state = OS_TCBList[ulIdx].state;
                OS_EXIT_CRITICAL();
                return state;
            }
        }
    }

    OS_EXIT_CRITICAL();
    return OS_TASK_UNINIT;
}

/*****************************************************************************
 Prototype    : OS_task_gettid
 Input        : void
 Output       : None
 Return Value : OS_TASK_ID
 Description  : Get current task's id
 Calls        :

*****************************************************************************/
OS_TASK_ID OS_task_gettid(void)
{
    OS_ISR_STATE    cpu_sr;
    OS_TASK_ID      tid;

    OS_ENTER_CRITICAL();
    tid = (OS_STATE_RUN == OS_info.state) ? OS_pCurrTask->id : OS_TASK_ID_INVALID;
    OS_EXIT_CRITICAL();
    return tid;
}

/*****************************************************************************
 Prototype    : OS_task_sleep
 Input        : ULONG32 ultimer, in unit  ms
 Output       : None
 Return Value : OS_RET_CODE
 Description  : task sleep
 Calls        :
        Task context is freely to call this function.
        Interrupt context is forbidden to call it.
*****************************************************************************/
OS_RET_CODE OS_task_sleep(ULONG32 ultimer)
{
    OS_ISR_STATE       cpu_sr;

    if ( OS_TICK_PERIOD > ultimer )
    {
        ultimer = 1;
    }
    else
    {
        ultimer = ultimer / OS_TICK_PERIOD;
    }

    OS_ENTER_CRITICAL();

    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NOT_READY;
    }

    OS_pCurrTask->state = OS_TASK_PEND;
    OS_pCurrTask->pend.timer.type = OS_TASK_PEND_TIMER;
    OS_pCurrTask->pend.timer.tick = ultimer;
    OS_EXIT_CRITICAL();
    OS_schedule();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_task_delete
 Input        : OS_TASK_ID task_id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Delete a task
     One task can be deleted by other tasks, not itseft
     So, no need to redo the schedule in OS_task_delete
 Calls        :
        Task context is freely to call this function.
        Interrupt context is forbidden to call it.
*****************************************************************************/
OS_RET_CODE OS_task_delete(OS_TASK_ID task_id)
{
    OS_ISR_STATE cpu_sr;

    /* idle task can not be deleted */
    if ((OS_TASK_CNT_MAX <= task_id) || (OS_IDLE_TASK_ID == task_id))
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_CRITICAL();

    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NOT_READY;
    }

    if (OS_pCurrTask->id == task_id)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_PARAM;
    }
    if (OS_TASK_UNINIT == OS_TCBList[task_id].state)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NOT_FOUND;
    }

    /* this task can not be deleted!!! */
    if (OS_TASK_OPT_UNDELETED & OS_TCBList[task_id].opt)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NOT_SUPPORT;
    }
    OS_EXIT_CRITICAL();

#ifdef OS_SUPPORT_MEM
    /* free memory this task has malloced */
    OS_mem_release(task_id);
#endif
    /* free mutex this task has taken */
    OS_mutex_release(task_id);
    /* free mailbox this task has created */
    OS_mailbox_release(task_id, OS_TICK_HZ);


    OS_ENTER_CRITICAL();
    OS_TCBList[task_id].state = OS_TASK_UNINIT;
    OS_TCBList[task_id].id = OS_TASK_ID_INVALID;
    OS_TCBList[task_id].name[0] = EOC;    
    OS_info.sched_period -= OS_TASK_WEIGHT(OS_TCBList[task_id].priority);
    OS_TCBList[task_id].original_priority = OS_TCBList[task_id].priority = OS_TASK_PRIORITY_LOWEST;
    OS_info.task_cnt--;
    OS_EXIT_CRITICAL();
#ifdef OS_SUPPORT_MEM
    OS_ENTER_MEM_CRITICAL();
    OS_ENTER_CRITICAL();
    if (OS_pCurrTask->opt & OS_TASK_OPT_MALLOC)
    {
        OS_mem_free_stk(task_id);
    }
    OS_EXIT_CRITICAL();
    OS_EXIT_MEM_CRITICAL();
#endif   
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_task_exit
 Input        : void
 Output       : None
 Return Value : void
 Description  : Task exits
 Calls        :
        Task context is freely to call this function.
        Interrupt context is forbidden to call it.
*****************************************************************************/
void OS_task_exit(void)
{
    OS_ISR_STATE cpu_sr;

    OS_ENTER_CRITICAL();
    /* cann't exit when os is in lock state */
    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return;
    }
    OS_EXIT_CRITICAL();
    
    /* free mutex this task has taken */
    OS_mutex_release(OS_pCurrTask->id);
    /* free mailbox this task has created */
    OS_mailbox_release(OS_pCurrTask->id, OS_TICK_HZ);
#ifdef OS_SUPPORT_MEM
    /* free memory this task has malloced */
    OS_mem_release(OS_pCurrTask->id);
#endif /* #ifdef OS_SUPPORT_MEM */

    OS_ENTER_CRITICAL();
    OS_info.sched_period -= OS_TASK_WEIGHT(OS_pCurrTask->priority);
    OS_pCurrTask->priority = 0;
    OS_pCurrTask->active_tick = OS_TASK_WEIGHT(OS_pCurrTask->priority);
    OS_EXIT_CRITICAL();

#ifdef OS_SUPPORT_MEM
    OS_ENTER_MEM_CRITICAL();
    OS_ENTER_CRITICAL();
    if (OS_pCurrTask->opt & OS_TASK_OPT_MALLOC)
    {
        OS_mem_free_stk(OS_pCurrTask->id); /* free stack must be putted at the end */
    }
    OS_pCurrTask->state = OS_TASK_UNINIT;
    OS_pCurrTask->id = OS_TASK_ID_INVALID;
    OS_pCurrTask->name[0] = EOC;
    OS_pCurrTask->original_priority = OS_pCurrTask->priority = OS_TASK_PRIORITY_LOWEST;
    OS_pCurrTask->active_tick = 0;
    OS_info.task_cnt--;
    OS_EXIT_CRITICAL();
    OS_EXIT_MEM_CRITICAL();
#else
    OS_ENTER_CRITICAL();
    OS_pCurrTask->state = OS_TASK_UNINIT;
    OS_pCurrTask->id = OS_TASK_ID_INVALID;
    OS_pCurrTask->name[0] = EOC;
    OS_pCurrTask->original_priority = OS_pCurrTask->priority = OS_TASK_PRIORITY_LOWEST;
    OS_pCurrTask->active_tick = 0;
    OS_info.task_cnt--;
    OS_EXIT_CRITICAL();
#endif /* #ifdef OS_SUPPORT_MEM */
    OS_schedule();
}

/*****************************************************************************
 Prototype    : OS_task_idle
 Input        : void *p_arg
 Output       : None
 Return Value : void
 Description  : Idle task
 Calls        :
        Only OS kernel can call this function
*****************************************************************************/
void OS_task_idle(void *p_arg)
{
    (void)p_arg; /* neutralize compiler warning */
    while (1);
}
#ifdef OS_SUPPORT_STAT
/*****************************************************************************
 Prototype    : OS_task_stat
 Input        : void *p_arg
 Output       : None
 Return Value : void
 Description  : get task stat info automatically
 Calls        :
        Only OS kernel can call this function
*****************************************************************************/
void OS_task_stat(void *p_arg)
{
    ULONG32 ulIdx;
    OS_TASK_ID  valid_cnt = 0;
    OS_ISR_STATE cpu_sr;

    while(1)
    {
        /* clear all statistic info */
        OS_ENTER_CRITICAL();
        for (ulIdx = 0, valid_cnt = 0; valid_cnt < OS_info.task_cnt; ulIdx++)
        {
            /* skip the task which has already been deleted, and the tasks being created */
            if ((OS_TASK_UNINIT != OS_TCBList[ulIdx].state)
                    && (OS_TASK_INIT != OS_TCBList[ulIdx].state))
            {
                valid_cnt++;
            }
            OS_TCBList[ulIdx].switch_counter = 0;
        }

        OS_EXIT_CRITICAL();

        OS_task_sleep(1000);
        OS_sched_period = OS_info.sched_period;
        OS_ctx_swt_tick = OS_info.ctx_swt_cnt;
        OS_sched_cycle = OS_info.sched_counter;
        OS_task_cnt = OS_info.task_cnt;

        OS_ENTER_CRITICAL();
        for (ulIdx = 0, valid_cnt = 0; valid_cnt < OS_task_cnt; ulIdx++)
        {
            /* skip the task which has already been deleted */
            if (OS_TASK_UNINIT == OS_TCBList[ulIdx].state)
            {
                continue;
            }
            OS_task_id[valid_cnt] = OS_TCBList[ulIdx].id;
            OS_task_usage[valid_cnt] = OS_TCBList[ulIdx].switch_counter;
            OS_task_pri[valid_cnt] = OS_TCBList[ulIdx].priority;
            OS_task_active_tick[valid_cnt] = OS_TCBList[ulIdx].active_tick;
            OS_task_stk_currsize[valid_cnt] = ((ULONG32)OS_TCBList[ulIdx].stk_bos
                    + (ULONG32)(OS_TCBList[ulIdx].stk_size - sizeof(OS_STK)) - (ULONG32)OS_TCBList[ulIdx].stk_tos)/sizeof(OS_STK) + 8;
            __strcpy(OS_task_name[valid_cnt], OS_TCBList[ulIdx].name);
            valid_cnt++;
        }
        OS_EXIT_CRITICAL();
    }
}

/*****************************************************************************
 Prototype    : OS_task_get_stat
 Input        : OS_TASK_ID aId[OS_TASK_CNT_MAX],
                OS_TASK_TIMER aUseage[OS_TASK_CNT_MAX],
                OS_TASK_NAME aName[OS_TASK_CNT_MAX],
                OS_TASK_PRIORITY apri[OS_TASK_CNT_MAX],
                OS_TASK_TIMER aActive_tick[OS_TASK_CNT_MAX]
                OS_SCHD_PERIOD *pPeriod
                OS_SCHD_PERIOD *pCycle
                OS_SCHD_PERIOD *pCtxStwCnt
 Output       : OS_TASK_ID    how many task left
 Return Value : void
 Description  : Output the statistic info
 Calls        :
        Task context is freely to call this function.
        Interrupt context is forbidden to call it.
*****************************************************************************/
OS_TASK_ID OS_task_get_stat(OS_TASK_ID aId[OS_TASK_CNT_MAX],
                                     OS_TASK_TIMER aUseage[OS_TASK_CNT_MAX],
                                      OS_STK_SIZE aStk_size[OS_TASK_CNT_MAX],
                                      OS_STK_SIZE aStk_curr_size[OS_TASK_CNT_MAX],
                                      OS_TASK_NAME aName[OS_TASK_CNT_MAX],
                                       OS_TASK_PRIORITY apri[OS_TASK_CNT_MAX],
                                        OS_TASK_TIMER aActive_tick[OS_TASK_CNT_MAX],
                                         OS_SCHD_PERIOD *pPeriod, OS_SCHD_PERIOD *pCycle,
                                          OS_SCHD_PERIOD *pCtxStwCnt)
{
    ULONG32         ulIdx;
    OS_TASK_ID      valid_cnt = 0;
    OS_ISR_STATE    cpu_sr;

    OS_ENTER_CRITICAL();
    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return 0;
    }
    OS_EXIT_CRITICAL();

    OS_lock_scheduler(true);
    *pPeriod = OS_sched_period;
    *pCycle = OS_sched_cycle;
    *pCtxStwCnt = OS_ctx_swt_tick;

    for (ulIdx = 0, valid_cnt = 0; valid_cnt < OS_task_cnt; ulIdx++)
    {
        /* skip the task which has already been deleted */
        if ((OS_TASK_UNINIT == OS_TCBList[ulIdx].state)
            || (OS_TASK_INIT == OS_TCBList[ulIdx].state))
        {
            continue;
        }
       aId[valid_cnt] = OS_task_id[valid_cnt];
       aUseage[valid_cnt] = OS_task_usage[valid_cnt];
       aStk_curr_size[valid_cnt] = OS_task_stk_currsize[valid_cnt];
       aStk_size[valid_cnt] = OS_TCBList[ulIdx].stk_size>>2;
       apri[valid_cnt] = OS_task_pri[valid_cnt];
       aActive_tick[valid_cnt] = OS_task_active_tick[valid_cnt];
       __strcpy(aName[valid_cnt], OS_task_name[valid_cnt]);
       valid_cnt++;
    }

    OS_lock_scheduler(false);
    return valid_cnt;
}
#endif /*#ifdef OS_SUPPORT_STAT*/

