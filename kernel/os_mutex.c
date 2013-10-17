/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_mutex.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS mutex source file

    History       :
        1.Date      : 2013/6/23
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#include <melon/melon.h>
#include <melon/os_core.h>
#include <melon/os_task.h>
#include <melon/os_mutex.h>

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/
extern OS_INFO OS_info;
extern OS_TCB  OS_TCBList[OS_TASK_CNT_MAX];
extern OS_TCB *OS_pCurrTask;
/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/
/* mutex list */
OS_MUTEX OS_mutex_list[OS_MUTEX_CNT_MAX];
/* total number of mutex */
OS_MUTEX_ID OS_mutex_cnt;

/*****************************************************************************
 Prototype    : OS_mutex_init
 Input        : void
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Init mutex block
 Calls        :


*****************************************************************************/
OS_RET_CODE OS_mutex_init(void)
{
    OS_ISR_STATE cpu_sr;
    ULONG32 ulIdx;

    OS_ENTER_CRITICAL();
    OS_mutex_cnt = 0;
    for (ulIdx = 0; ulIdx < OS_MUTEX_CNT_MAX; ulIdx++)
    {
        OS_mutex_list[ulIdx].id = OS_MUTEX_CNT_MAX+1;
        OS_mutex_list[ulIdx].state= OS_MUTEX_UNINIT;
#ifdef OS_MUTEX_SUPPORT_NAME
        OS_mutex_list[ulIdx].name[0] = EOC;
#endif
        OS_mutex_list[ulIdx].owner = OS_TASK_ID_INVALID;
        OS_mutex_list[ulIdx].wait_taskcnt = 0;
        OS_mutex_list[ulIdx].flag = 0;
    }
    OS_EXIT_CRITICAL();

    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mutex_create
 Input        : OS_MUTEX_NAME name
                OS_MUTEX_FLAG opt
                OS_MUTEX_ID *pMutex_id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Create a mutex
    If you don't need mutex with name, undef macro OS_MUTEX_SUPPORT_NAME,
    And pass NULL to args.name;
    If you compile OS with macro OS_MUTEX_SUPPORT_NAME defined, you must
    pass a valid char* pointer to args.name;
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mutex_create(OS_MUTEX_NAME name, OS_MUTEX_OPT opt, OS_MUTEX_ID *pMutex_id)
{
    OS_ISR_STATE cpu_sr;
    OS_MUTEX_ID mutex_id;
#ifdef OS_MUTEX_SUPPORT_NAME
    OS_MUTEX_ID deal_cnt;
#endif

    OS_ENTER_CRITICAL();
    if (OS_mutex_cnt == OS_MUTEX_CNT_MAX)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NO_MEM;
    }

    for (mutex_id = 0 ; mutex_id < OS_MUTEX_CNT_MAX; mutex_id++)
    {
        if (OS_MUTEX_UNINIT == OS_mutex_list[mutex_id].state)
        {
            break;
        }
    }
    __AFFIRM(OS_MUTEX_UNINIT == OS_mutex_list[mutex_id].state);

    OS_mutex_list[mutex_id].state = OS_MUTEX_INIT;
    OS_mutex_list[mutex_id].id = mutex_id;
    OS_mutex_list[mutex_id].owner = OS_TASK_ID_INVALID;
    OS_mutex_list[mutex_id].wait_taskcnt = 0;

#ifdef OS_MUTEX_SUPPORT_NAME
    if (NULL != name)
    {
        for (deal_cnt = 0; EOC != (OS_mutex_list[mutex_id].name[deal_cnt] = name[deal_cnt]); deal_cnt++)
        {
            if (OS_MUTEX_NAME_LEN_MAX == deal_cnt)
            {
                OS_mutex_list[mutex_id].name[OS_MUTEX_NAME_LEN_MAX - 1] = EOC;
                break;
            }
        }
    }
    else
    {
        OS_EXIT_CRITICAL();
        return OS_RET_PARAM;
    }
#endif /* #ifdef OS_MUTEX_SUPPORT_NAME */
    *pMutex_id = mutex_id;
    OS_mutex_list[mutex_id].state = OS_MUTEX_UNLOCK;
    OS_mutex_cnt++;
    OS_EXIT_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mutex_trylock
 Input        : OS_MUTEX_ID mutex_id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Try to lock a mutex
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mutex_trylock(OS_MUTEX_ID mutex_id)
{
    OS_ISR_STATE cpu_sr;

    OS_ENTER_CRITICAL();

    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NOT_READY;
    }

    if (OS_mutex_cnt <= mutex_id)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_PARAM;
    }

    switch (OS_mutex_list[mutex_id].state)
    {
        case OS_MUTEX_UNINIT:
            OS_EXIT_CRITICAL();
            return OS_RET_UNINIT;
        case OS_MUTEX_UNLOCK:
            goto __take_mutex;
        case OS_MUTEX_LOCKED:
            OS_EXIT_CRITICAL();
            return OS_RET_BUSY;
        default:
            OS_EXIT_CRITICAL();
            __AFFIRM(false);
    }

__take_mutex:
    OS_mutex_list[mutex_id].state = OS_MUTEX_LOCKED;
    OS_mutex_list[mutex_id].owner = OS_pCurrTask->id;
    OS_EXIT_CRITICAL();
    return OS_RET_SUCCESS;
}


/*****************************************************************************
 Prototype    : OS_mutex_lock
 Input        : OS_MUTEX_ID mutex_id
                OS_MUTEX_TIMER timeout, how many tick before timeout, 0 means forever
                OS_MUTEX_FLAG opt
 Output       : None
 Return Value : OS_RET_CODE
 Description  : lock a mutex
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mutex_lock(OS_MUTEX_ID mutex_id, OS_MUTEX_TIMER timeout, OS_MUTEX_OPT opt)
{
    OS_ISR_STATE cpu_sr;

    /* reserved parameter */
    (void)opt;

    OS_ENTER_CRITICAL();

    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NOT_READY;
    }

    if (OS_mutex_cnt <= mutex_id)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_PARAM;
    }

    switch (OS_mutex_list[mutex_id].state)
    {
        case OS_MUTEX_UNINIT:
            OS_EXIT_CRITICAL();
            return OS_RET_UNINIT;
        case OS_MUTEX_UNLOCK:
            goto __take_mutex;
        case OS_MUTEX_LOCKED:
            if (OS_mutex_list[mutex_id].owner != OS_pCurrTask->id)
            {
                goto __join_waitlist;
            }
            else
            {
                OS_EXIT_CRITICAL();
                return OS_RET_SUCCESS;
            }
        default:
            OS_EXIT_CRITICAL();
            __AFFIRM(false);
    }

__take_mutex:
    OS_mutex_list[mutex_id].state = OS_MUTEX_LOCKED;
    OS_mutex_list[mutex_id].owner = OS_pCurrTask->id;
    OS_EXIT_CRITICAL();
    return OS_RET_SUCCESS;

__join_waitlist:
    /* a. find an aviable pos in wait list */
    if (OS_MUTEX_WAITLIST_CNT_MAX == OS_mutex_list[mutex_id].wait_taskcnt)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_FULL;
    }

    OS_mutex_list[mutex_id].wait_taskcnt++;

    OS_pCurrTask->state = OS_TASK_PEND;
    OS_pCurrTask->pend.mutex.type = OS_TASK_PEND_MUTEX;
    OS_pCurrTask->pend.mutex.id = mutex_id;
    OS_pCurrTask->pend.mutex.tick = timeout;
    OS_pCurrTask->pend.mutex.ret = OS_MUTEX_PEND_ING;
    OS_TCBList[OS_mutex_list[mutex_id].owner].priority =
        OS_HIGHER_PRIORITY(OS_TCBList[OS_mutex_list[mutex_id].owner].priority, OS_pCurrTask->priority);
    OS_TCBList[OS_mutex_list[mutex_id].owner].active_tick += OS_pCurrTask->active_tick;
    OS_pCurrTask->active_tick = 0;

    OS_EXIT_CRITICAL();

    OS_schedule();

    OS_ENTER_CRITICAL();
    /* refresh the wait list */

    OS_mutex_list[mutex_id].wait_taskcnt--;

    /* get the mutex before timeout */
    if (OS_MUTEX_PEND_OK == OS_pCurrTask->pend.mutex.ret)
    {
        OS_pCurrTask->pend.mutex.ret = OS_MUTEX_PEND_INIT;
        OS_EXIT_CRITICAL();
        return OS_RET_SUCCESS;
    }
    else if (OS_MUTEX_PEND_TIMEOUT == OS_pCurrTask->pend.mutex.ret)
    {
        OS_pCurrTask->pend.mutex.ret = OS_MUTEX_PEND_INIT;
        OS_EXIT_CRITICAL();
        return OS_RET_TIMEOUT;
    }
    else
    {
        OS_EXIT_CRITICAL();
#ifdef MELON_DBG
        __AFFIRM(false);
#else
        return OS_RET_PARAM;
#endif
    }
    /*never come here*/
}

/*****************************************************************************
 Prototype    : OS_mutex_unlock
 Input        : OS_MUTEX_ID mutex_id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Release a mutex
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mutex_unlock(OS_MUTEX_ID mutex_id)
{
    OS_ISR_STATE cpu_sr;
    OS_MUTEX_ID usIdx;
    OS_MUTEX_ID deal_cnt;
    OS_TASK_ID  candt_task_id_pri = OS_IDLE_TASK_ID;

    OS_TASK_ID  candt_task_id_tick = OS_IDLE_TASK_ID;
    OS_TASK_PRIORITY candt_pri = OS_TASK_PRIORITY_LOWEST;
    OS_TASK_TIMER candt_tick = 0;


    OS_ENTER_CRITICAL();

    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NOT_READY;
    }

    if (OS_mutex_cnt <= mutex_id)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_PARAM;
    }
    switch (OS_mutex_list[mutex_id].state)
    {
        case OS_MUTEX_UNINIT:
            OS_EXIT_CRITICAL();
            return OS_RET_UNINIT;
        case OS_MUTEX_UNLOCK:
            OS_EXIT_CRITICAL();
            return OS_RET_PARAM;
        case OS_MUTEX_LOCKED:
            /*
                当一个task exit时, OS_pCurrTask->id已经设置为invalid,
                需要支持此时继续释放.
            */
            if ((OS_mutex_list[mutex_id].owner == OS_pCurrTask->id)
                 || ((OS_TASK_ID_INVALID == OS_pCurrTask->id) && (OS_TASK_UNINIT == OS_pCurrTask->state)))
            {
                /* 在优先级翻转的情况下要恢复原始优先级 */
                if (OS_pCurrTask->priority != OS_pCurrTask->original_priority)
                {
                    OS_pCurrTask->priority = OS_pCurrTask->original_priority;
                }
                goto __wake_up_pool_task;
            }
            else
            {
                OS_EXIT_CRITICAL();
                return OS_RET_PARAM;
            }
        default:
            OS_EXIT_CRITICAL();
#ifdef MELON_DBG
            __AFFIRM(false);
#else
            return OS_RET_PARAM;
#endif

    }

/* the poor guys are waiting for this mutex, let's wake up one of them here */
__wake_up_pool_task:
    /* no more task waiting for this mutex */
    if (0 == OS_mutex_list[mutex_id].wait_taskcnt)
    {
       OS_mutex_list[mutex_id].owner = OS_TASK_ID_INVALID;
       OS_mutex_list[mutex_id].state = OS_MUTEX_UNLOCK;
       OS_EXIT_CRITICAL();
       OS_schedule();
       return OS_RET_SUCCESS;
    }

    /* find a lucky guy */
    for (usIdx = 0, deal_cnt = 0; deal_cnt < OS_mutex_list[mutex_id].wait_taskcnt; usIdx++ )
    {
        if ((OS_TASK_PEND == OS_TCBList[usIdx].state) && (OS_TASK_PEND_MUTEX == OS_TCBList[usIdx].pend.mutex.type))
        {
            if (mutex_id == OS_TCBList[usIdx].pend.mutex.id)
            {
                if (OS_TCBList[usIdx].active_tick > candt_tick)
                {
                    candt_tick = OS_TCBList[usIdx].active_tick;
                    candt_task_id_tick = OS_TCBList[usIdx].id;
                }
                else if (OS_TCBList[usIdx].priority < candt_pri)
                {
                    candt_pri = OS_TCBList[usIdx].priority;
                    candt_task_id_pri = OS_TCBList[usIdx].id;
                }
                deal_cnt++;
            }
        }

    }

    if (OS_IDLE_TASK_ID == candt_task_id_tick)
    {
        candt_task_id_tick = candt_task_id_pri;
    }

    /* OK, we got the lucky guy, wake up him now */
    OS_TCBList[candt_task_id_tick].pend.mutex.ret = OS_MUTEX_PEND_OK;
    OS_TCBList[candt_task_id_tick].state = OS_TASK_READY;
    /* change the mutex ower */
    OS_mutex_list[mutex_id].owner = candt_task_id_tick; /* still locked */

    OS_EXIT_CRITICAL();
    OS_schedule();

    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mutex_free
 Input        : OS_TASK_ID task_id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Free the mutexs this task has taken
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mutex_release(OS_TASK_ID task_id)
{
    OS_ISR_STATE cpu_sr;
    OS_MUTEX_ID usIdx;
    OS_MUTEX_ID deal_cnt1, deal_cnt2;
    OS_TASK_ID  candt_task_id_pri = OS_IDLE_TASK_ID;
    OS_TASK_ID  candt_task_id_tick = OS_IDLE_TASK_ID;
    OS_TASK_PRIORITY candt_pri = OS_TASK_PRIORITY_LOWEST;
    OS_TASK_TIMER candt_tick = 0;
    OS_MUTEX_ID     mutex_id = 0;

    OS_ENTER_CRITICAL();

    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_NOT_READY;
    }

    if (task_id >= OS_TASK_CNT_MAX)
    {
        OS_EXIT_CRITICAL();
        return OS_RET_PARAM;
    }
    /* this task is waiting a mutex, need to refresh wait list */
    if ((OS_TASK_PEND == OS_TCBList[task_id].state)
        && (OS_TASK_PEND_MUTEX == OS_TCBList[task_id].pend.mutex.type))
    {
        /* find the target mutex */
        usIdx = OS_TCBList[task_id].pend.mutex.id;

        if (OS_mutex_list[usIdx].wait_taskcnt > 0)
        {
            OS_mutex_list[usIdx].wait_taskcnt--;
        }
    }

    for (usIdx = 0, deal_cnt1 = 0; deal_cnt1 < OS_mutex_cnt; usIdx++ )
    {
        if (task_id == OS_mutex_list[usIdx].owner)
        {
            mutex_id = OS_mutex_list[usIdx].id;
            /* no more task waiting for this mutex */
            if (0 == OS_mutex_list[mutex_id].wait_taskcnt)
            {
               OS_mutex_list[mutex_id].owner = OS_TASK_ID_INVALID;
               OS_mutex_list[mutex_id].state = OS_MUTEX_UNLOCK;
               continue; /* find the next mutex this task taken */
            }

            /* find a lucky guy */
            for (usIdx = 0, deal_cnt2 = 0; deal_cnt2 < OS_mutex_list[mutex_id].wait_taskcnt; usIdx++ )
            {
                if ((OS_TASK_PEND == OS_TCBList[usIdx].state)
                    && (OS_TASK_PEND_MUTEX == OS_TCBList[usIdx].pend.mutex.type))
                {
                    if (mutex_id == OS_TCBList[usIdx].pend.mutex.id)
                    {
                        if (OS_TCBList[usIdx].active_tick > candt_tick)
                        {
                            candt_tick = OS_TCBList[usIdx].active_tick;
                            candt_task_id_tick = OS_TCBList[usIdx].id;
                        }
                        else if (OS_TCBList[usIdx].priority < candt_pri)
                        {
                            candt_pri = OS_TCBList[usIdx].priority;
                            candt_task_id_pri = OS_TCBList[usIdx].id;
                        }
                        deal_cnt2++;
                    }
                }
            }

            if (OS_IDLE_TASK_ID == candt_task_id_tick)
            {
                candt_task_id_tick = candt_task_id_pri;
            }

            /* OK, we got the lucky guy, wake up him now */
            OS_TCBList[candt_task_id_tick].pend.mutex.ret = OS_MUTEX_PEND_OK;
            OS_TCBList[candt_task_id_tick].state = OS_TASK_READY;
            /* change the mutex ower */
            OS_mutex_list[mutex_id].owner = candt_task_id_tick; /* still locked */

        }

        if (OS_MUTEX_UNINIT != OS_mutex_list[usIdx].state)
        {
            deal_cnt1++;
        }
    }
    OS_EXIT_CRITICAL();

    return OS_RET_SUCCESS;
}

