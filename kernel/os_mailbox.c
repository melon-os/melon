/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_mailbox.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/23
    Last Modified :
    Description   : Melon OS mailbox source file

    History       :
        1.Date        : 2013/6/23
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#include <melon/melon.h>
#include <melon/os_core.h>
#include <melon/os_task.h>
#include <melon/os_mutex.h>
#include <melon/os_mailbox.h>

/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/
extern OS_INFO OS_info;
extern OS_TCB  OS_TCBList[OS_TASK_CNT_MAX];
extern OS_TCB *OS_pCurrTask;
/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/
/* mailbox list */
OS_MBOX     OS_mailbox[OS_MBOX_CNT_MAX];
/* total number of mailbox */
OS_MBOX_ID  OS_mailbox_cnt;
/* mutex id which is used in mailbox block */
#ifndef OS_MBOX_SUPPORT_INTER
OS_MUTEX_ID OS_mbox_mutex_id;
#endif
/*****************************************************************************
 Prototype    : OS_mailbox_init
 Input        : void
 Output       : None
 Return Value : OS_RET_CODE
 Description  :
        Initialize mailbox
 Calls        :
        Only OS level context could call this function.
*****************************************************************************/
OS_RET_CODE OS_mailbox_init(void)
{
    OS_ISR_STATE    cpu_sr;
    OS_MBOX_ID      usIdx;
    OS_MSG_ID       usIdy;
    OS_RET_CODE     ret;

    (void)ret; /* neutralize compiler warning */

    OS_ENTER_CRITICAL();
#ifndef OS_MBOX_SUPPORT_INTER
    ret = OS_mutex_create("MboxMngMutexId", 0, &OS_mbox_mutex_id);
    __AFFIRM(OS_RET_SUCCESS == ret);
#endif
    OS_mailbox_cnt = 0;
    for (usIdx = 0; usIdx < OS_MBOX_CNT_MAX; usIdx++ )
    {
        OS_mailbox[usIdx].id = OS_MBOX_ID_INVALID;
        OS_mailbox[usIdx].state = OS_MBOX_FREE;
        OS_mailbox[usIdx].msg_cnt = 0;
        OS_mailbox[usIdx].owner = OS_TASK_ID_INVALID;

        for (usIdy = 0; usIdy < OS_MBOX_MSG_CNT_MAX; usIdy++ )
        {
            OS_mailbox[usIdx].msg[usIdy].id = OS_MBOX_MSG_ID_MAX;
            OS_mailbox[usIdx].msg[usIdy].dst = OS_TASK_ID_INVALID;
            OS_mailbox[usIdx].msg[usIdy].src = OS_TASK_ID_INVALID;
            OS_mailbox[usIdx].msg[usIdy].tmo_timer = 0;
            OS_mailbox[usIdx].msg[usIdy].pexd = NULL;
            OS_mailbox[usIdx].msg[usIdy].content = 0;
        }
    }
    OS_EXIT_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mailbox_create
 Input        : OS_MBOX_ID *pMboxId, pointer of mailbox id
 Output       : None
 Return Value : OS_RET_CODE
 Description  :
        Create a mailbox.
        Each mbox(mailbox's abbr) has only one creator while can be used by
        two(one poster and one receiver) or more tasks.
        Any one of these tasks can be the poster or receiver.
 Calls        :
        Only can be called in task context.
        Interrupt context is forbidden to call this function.
*****************************************************************************/
OS_RET_CODE OS_mailbox_create(OS_MBOX_ID *pMboxId)
{
#ifdef OS_MBOX_SUPPORT_INTER
    OS_ISR_STATE    cpu_sr;
#endif
    OS_MBOX_ID      usIdx;


    if (NULL == pMboxId)
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_MBOX_CRITICAL();
    if (OS_MBOX_CNT_MAX == OS_mailbox_cnt)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_FAIL;
    }

    for (usIdx = 0; usIdx < OS_MBOX_CNT_MAX; usIdx++ )
    {
        if (OS_MBOX_FREE == OS_mailbox[usIdx].state)
        {
            OS_mailbox[usIdx].state = OS_MBOX_BUSY;
            OS_mailbox[usIdx].msg_cnt = 0;
            OS_mailbox[usIdx].owner = OS_pCurrTask->id;
            *pMboxId = OS_mailbox[usIdx].id = usIdx;
            break;
        }
    }
    OS_mailbox_cnt++;
    __AFFIRM(usIdx < OS_MBOX_CNT_MAX);

    OS_EXIT_MBOX_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mailbox_delete
 Input        : OS_MBOX_ID mbox_id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Delete a mailbox
        If you pass the mbox which is still holding some messages,
        this function will not return to the caller until all the messages
        have been read or the tmo_tick decreases to 0.
 Calls        :
        Only can be called in task context;
        Interrupt context is forbidden to call this function.
*****************************************************************************/
OS_RET_CODE OS_mailbox_delete(OS_MBOX_ID mbox_id, OS_TASK_TIMER tmo_tick)
{
#ifdef OS_MBOX_SUPPORT_INTER
    OS_ISR_STATE    cpu_sr;
#endif
    ULONG32         ulIdy;
    ULONG32         ulReleaseCnt;

    OS_ENTER_MBOX_CRITICAL();

    if (mbox_id >= OS_MBOX_CNT_MAX)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_PARAM;
    }

    ulReleaseCnt = 0;
    if (OS_MBOX_BUSY == OS_mailbox[mbox_id].state)
    {
        /* delete mailbox */
        while ((0 != OS_mailbox[mbox_id].msg_cnt) && (0 != tmo_tick--))
        {
            OS_EXIT_MBOX_CRITICAL();
            OS_task_sleep(OS_TICK_PERIOD);
            OS_ENTER_MBOX_CRITICAL();
        }
    }

    OS_mailbox[mbox_id].id = 0;
    OS_mailbox[mbox_id].msg_cnt = 0;
    OS_mailbox[mbox_id].owner = OS_TASK_ID_INVALID;
    OS_mailbox[mbox_id].state = OS_MBOX_FREE;
    for (ulIdy = 0; ulIdy < OS_MBOX_MSG_CNT_MAX; ulIdy++ )
    {
        OS_mailbox[mbox_id].msg[ulIdy].id = OS_MBOX_MSG_ID_MAX;
        OS_mailbox[mbox_id].msg[ulIdy].dst = OS_TASK_ID_INVALID;
        OS_mailbox[mbox_id].msg[ulIdy].src = OS_TASK_ID_INVALID;
        OS_mailbox[mbox_id].msg[ulIdy].tmo_timer = 0;
        OS_mailbox[mbox_id].msg[ulIdy].pexd = NULL;
        OS_mailbox[mbox_id].msg[ulIdy].content = 0;
    }

    ulReleaseCnt = 1;
    OS_mailbox_cnt -= ulReleaseCnt;
    OS_EXIT_MBOX_CRITICAL();

    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mailbox_release
 Input        : OS_TASK_ID task_id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Release a mailbox created by the specific task
 Calls        :
        Only OS level context could call this function.
*****************************************************************************/
OS_RET_CODE OS_mailbox_release(OS_TASK_ID task_id, OS_TASK_TIMER tmo_tick)
{
#ifdef OS_MBOX_SUPPORT_INTER
    OS_ISR_STATE    cpu_sr;
#endif
    ULONG32         ulIdx, ulIdy;
    ULONG32         ulChkCnt;
    ULONG32         ulReleaseCnt;

    OS_ENTER_MBOX_CRITICAL();

    if (task_id >= OS_TASK_CNT_MAX)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_PARAM;
    }

    for (ulIdx = 0, ulChkCnt = 0, ulReleaseCnt = 0; ulChkCnt < OS_mailbox_cnt; ulIdx++)
    {
        if (OS_MBOX_BUSY == OS_mailbox[ulIdx].state)
        {
            if (OS_mailbox[ulIdx].owner == task_id)
            {
                /* delete mailbox */
                while ((0 != OS_mailbox[ulIdx].msg_cnt) && (0 != tmo_tick--))
                {
                    OS_EXIT_MBOX_CRITICAL();
                    OS_task_sleep(OS_TICK_PERIOD);
                    OS_ENTER_MBOX_CRITICAL();
                }
                OS_mailbox[ulIdx].id = 0;
                OS_mailbox[ulIdx].msg_cnt = 0;
                OS_mailbox[ulIdx].owner = OS_TASK_ID_INVALID;
                OS_mailbox[ulIdx].state = OS_MBOX_FREE;

                for (ulIdy = 0; ulIdy < OS_MBOX_MSG_CNT_MAX; ulIdy++ )
                {
                    OS_mailbox[ulIdx].msg[ulIdy].id = OS_MBOX_MSG_ID_MAX;
                    OS_mailbox[ulIdx].msg[ulIdy].dst = OS_TASK_ID_INVALID;
                    OS_mailbox[ulIdx].msg[ulIdy].src = OS_TASK_ID_INVALID;
                    OS_mailbox[ulIdx].msg[ulIdy].tmo_timer = 0;
                    OS_mailbox[ulIdx].msg[ulIdy].pexd = NULL;
                    OS_mailbox[ulIdx].msg[ulIdy].content = 0;
                }
                ulReleaseCnt++;
            }
            ulChkCnt++;
        }
    }
    OS_mailbox_cnt -= ulReleaseCnt;

    OS_EXIT_MBOX_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mailbox_post
 Input        : OS_MBOX_ID mbox_id
                OS_MBOX_MSG *pMsg, pMsg->id is no need to pass in
                OS_MBOX_OPT opt
 Output       : None
 Return Value : OS_RET_CODE
 Description  :
        Send out a message to other tasks or the poster itself.
        If the destination task is pending at this mailbox,
        It will be informed here, and waked up by scheduler.

        If the destination task is not pending at this mailbox,
        The msg will be added into mailbox and return to caller directly.
 Calls        :
        Task context is freely to call this function.
        If you want to call it in interrupt context, you should define macro OS_MBOX_SUPPORT_INTER.
*****************************************************************************/
OS_RET_CODE OS_mailbox_post(OS_MBOX_ID mbox_id, OS_MBOX_MSG *pMsg, OS_MSG_OPT opt)
{
#ifdef OS_MBOX_SUPPORT_INTER
    OS_ISR_STATE    cpu_sr;
#endif

    OS_MBOX_ID      usIdx;

    if (NULL == pMsg)
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_MBOX_CRITICAL();

    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_NOT_READY;
    }

    if (OS_MBOX_ID_INVALID <= mbox_id)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_PARAM;
    }

    if (OS_MBOX_BUSY != OS_mailbox[mbox_id].state)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_PARAM;
    }

    if (OS_MBOX_MSG_CNT_MAX == OS_mailbox[mbox_id].msg_cnt)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_PARAM;
    }

    /* only invalid task can receive msg */
    if ((pMsg->dst >= OS_TASK_CNT_MAX) || (OS_TASK_UNINIT == OS_TCBList[pMsg->dst].state)
        || (OS_TASK_INIT == OS_TCBList[pMsg->dst].state))
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_PARAM;
    }

    usIdx = 0;
    while (OS_MBOX_MSG_ID_MAX != OS_mailbox[mbox_id].msg[usIdx].id)
    {
        usIdx++;
    }
    __AFFIRM(usIdx < OS_MBOX_MSG_CNT_MAX);

    OS_mailbox[mbox_id].msg[usIdx].id = OS_info.ctx_swt_cnt; /* sequence id, debug trace use*/
    OS_mailbox[mbox_id].msg[usIdx].dst = pMsg->dst;
    OS_mailbox[mbox_id].msg[usIdx].src = OS_pCurrTask->id;
    OS_mailbox[mbox_id].msg[usIdx].tmo_timer = pMsg->tmo_timer;
    OS_mailbox[mbox_id].msg[usIdx].pexd = pMsg->pexd;
    OS_mailbox[mbox_id].msg[usIdx].content = pMsg->content;
    OS_mailbox[mbox_id].msg_cnt++;

    /* check if the dst task is pended by mailbox */
    if (OS_TASK_PEND == OS_TCBList[pMsg->dst].state)
    {
        if (OS_TASK_PEND_MBOX == OS_TCBList[pMsg->dst].pend.mbox.type)
        {
            if (mbox_id == OS_TCBList[pMsg->dst].pend.mbox.id)
            {
//                OS_TCBList[pMsg->dst].pend.mbox.msg_idx = usIdx;
                OS_TCBList[pMsg->dst].pend.mbox.tick = 0;
                OS_TCBList[pMsg->dst].pend.mbox.ret = OS_MBOX_PEND_OK;
                OS_TCBList[pMsg->dst].state = OS_TASK_READY;
                OS_EXIT_MBOX_CRITICAL();
                OS_schedule();
                return OS_RET_SUCCESS;
            }
        }
    }
    OS_EXIT_MBOX_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mailbox_recv
 Input        : OS_MBOX_ID mbox_id
                OS_MBOX_MSG *pMsg
                OS_MBOX_TIMER tmo_timer
 Output       : None
 Return Value : OS_RET_CODE
 Description  :
        Receive a message from the mailbox
        If there is one or more msg in mailbox whose destination is the caller,
        It will return directly after read the msg.
        If there is no msg in mailbox whose destination task id is the caller,
        The caller will pend until gets a msg or time out occurs.
 Calls        :
        Task context is freely to call this function.
        Interrupt context is forbidden to call it.

*****************************************************************************/
OS_RET_CODE OS_mailbox_recv(OS_MBOX_ID mbox_id , OS_MBOX_MSG *pMsg, OS_MSG_TIMER tmo_timer)
{
#ifdef OS_MBOX_SUPPORT_INTER
    OS_ISR_STATE    cpu_sr;
#endif
    OS_MBOX_ID      usIdx;
    OS_MBOX_ID      deal_cnt;

    if (NULL == pMsg)
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_MBOX_CRITICAL();

    if (OS_STATE_RUN != OS_info.state)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_NOT_READY;
    }

    if (OS_MBOX_ID_INVALID <= mbox_id)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_PARAM;
    }

    if (OS_MBOX_BUSY != OS_mailbox[mbox_id].state)
    {
        OS_EXIT_MBOX_CRITICAL();
        return OS_RET_PARAM;
    }

    if (0 == OS_mailbox[mbox_id].msg_cnt) /* pend this task by mailbox */
    {
        OS_pCurrTask->state = OS_TASK_PEND;
        OS_pCurrTask->pend.mbox.type = OS_TASK_PEND_MBOX;
        OS_pCurrTask->pend.mbox.tick = tmo_timer;
        OS_pCurrTask->pend.mbox.id = mbox_id;
        OS_pCurrTask->pend.mbox.ret = OS_MBOX_PEND_ING;

        OS_EXIT_MBOX_CRITICAL();
        OS_schedule();

        OS_ENTER_MBOX_CRITICAL();
        if (OS_MBOX_PEND_OK == OS_pCurrTask->pend.mbox.ret)
        {
            OS_pCurrTask->pend.mbox.ret = OS_MBOX_PEND_INIT;
            OS_pCurrTask->pend.mbox.id = OS_MBOX_ID_INVALID;
        }
        else /* timeout */
        {
            /* have not found msg to this task */
            OS_EXIT_MBOX_CRITICAL();
            return OS_RET_TIMEOUT;
        }
    }

    /* have not found msg to this task */

    __AFFIRM(OS_mailbox[mbox_id].msg_cnt > 0);


   for (usIdx = 0, deal_cnt = 0; deal_cnt < OS_mailbox[mbox_id].msg_cnt; usIdx++)
   {
        if (OS_MBOX_MSG_ID_MAX == OS_mailbox[mbox_id].msg[usIdx].id)
        {
            continue;
        }

        if (OS_pCurrTask->id == OS_mailbox[mbox_id].msg[usIdx].dst)
        {
            goto __read_msg;
        }
       deal_cnt++;
   }
   /* have not found msg to this task */
   if (deal_cnt == OS_mailbox[mbox_id].msg_cnt)
   {
       OS_EXIT_MBOX_CRITICAL();
       return OS_RET_EMPTY;
   }

__read_msg:
    pMsg->id = OS_mailbox[mbox_id].msg[usIdx].id;
    pMsg->dst =OS_mailbox[mbox_id].msg[usIdx].dst;
    pMsg->src = OS_mailbox[mbox_id].msg[usIdx].src;
    pMsg->pexd = OS_mailbox[mbox_id].msg[usIdx].pexd;
    pMsg->content = OS_mailbox[mbox_id].msg[usIdx].content;

    OS_mailbox[mbox_id].msg[usIdx].id = OS_MBOX_MSG_ID_MAX;
    OS_mailbox[mbox_id].msg_cnt--;

    OS_EXIT_MBOX_CRITICAL();
    return OS_RET_SUCCESS;
}

/*
    If you want to call it in interrupt context,
    macro OS_MBOX_SUPPORT_INTER shoule be defined in melon_cfg.h
*/
