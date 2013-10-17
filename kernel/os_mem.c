/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_mem.c
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/26
    Last Modified :
    Description   : Melon OS memory management source file

    History       :
        1.Date      : 2013/6/30
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#include <melon/melon.h>
#include <melon/os_task.h>
#include <melon/os_mutex.h>
#include <melon/os_mem.h>
#include <stdio.h>

#ifdef OS_SUPPORT_MEM
/*----------------------------------------------*
 * external variables                           *
 *----------------------------------------------*/
extern OS_TCB  OS_TCBList[OS_TASK_CNT_MAX];
extern OS_TCB *OS_pCurrTask;
/*----------------------------------------------*
 * project-wide global variables                *
 *----------------------------------------------*/
/* memory pool managed by this module */
__align(8) UCHAR8 OS_mem_pool[OS_MEM_SIZE*1024] = {0};
/* memory list of each memory type */
OS_MEM_LIST OS_mem_list[MEM_TYPE_CNT];
/* memory control item list */
OS_MEM_CTRL OS_mem_ctrl[OS_MEM_CTRL_CNT];
/* mutex id used in this module */
OS_MUTEX_ID OS_mem_mutex_id;
/* size of each memory type */
USHORT16 OS_mem_type[MEM_TYPE_CNT] = {
    8,
    12,
    16,
    24,
    32,
    48,
    64,
    96,
    128,
    192,
    256,
    384,
    512,
    768,
    1024,
    1536,
    2048,
    3072,
    4096,
};

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
static OS_RET_CODE __mem_partiion(ULONG32 base_addr, ULONG32 size);
static OS_RET_CODE __mem_add(OS_MEM_TYPE type, ULONG32 base_addr);
static OS_RET_CODE __mem_del(OS_MEM_TYPE ori_type, OS_MEM_TYPE type, OS_MEM_CTRL **pplist);
static OS_RET_CODE __mem_move(OS_MEM_TYPE type, OS_MEM_CTRL *plist);
static OS_MEM_TYPE __mem_align(ULONG32 size);
static BOOL __decrease_to_zero(ULONG32 size, OS_MEM_TYPE type);

/*****************************************************************************
 Prototype    : OS_mem_init
 Input        : void
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Memory management initialization
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mem_init(void)
{
    OS_ISR_STATE    cpu_sr;
    OS_RET_CODE     ret;
    ULONG32         ulIdx;

    (void)ret; /* neutralize compiler warning */
    OS_ENTER_CRITICAL();
    ret = OS_mutex_create("MemMngMutexId", 0, &OS_mem_mutex_id);
    __AFFIRM(OS_RET_SUCCESS == ret);

    for (ulIdx = 0; ulIdx < OS_MEM_CTRL_CNT; ulIdx++)
    {
        OS_mem_ctrl[ulIdx].state = MEM_INIT;
        OS_mem_ctrl[ulIdx].type = MEM_TYPE_INIT;
        OS_mem_ctrl[ulIdx].prev = OS_mem_ctrl[ulIdx].next = NULL;
        OS_mem_ctrl[ulIdx].addr = 0;
        OS_mem_ctrl[ulIdx].owner = OS_TASK_ID_INVALID;
    }

    for (ulIdx = 0; ulIdx < OS_MEM_CTRL_CNT; ulIdx++)
    {
        OS_mem_list[ulIdx].head = NULL;
        OS_mem_list[ulIdx].cnt = 0;
    }

    ret = __mem_partiion((ULONG32)(&OS_mem_pool[0]), sizeof(OS_mem_pool));
    __AFFIRM(OS_RET_SUCCESS == ret);

    OS_EXIT_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mem_query
 Input        : ULONG32 type_cnt[MEM_TYPE_CNT]
                ULONG32 *pctr_cnt
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Get the avariable memory size
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mem_query(ULONG32 type_cnt[MEM_TYPE_CNT], ULONG32 *pctr_cnt)
{
    ULONG32 ulIdx;

    /* 可以只传入一个有效的指针, 即只查询一种信息 */
    if ((NULL == type_cnt) && (NULL == pctr_cnt))
    {
        return OS_RET_PARAM;
    }

    OS_ENTER_MEM_CRITICAL();
    if (NULL!= type_cnt)
    {
        for (ulIdx = MEM_TYPE_MIN; ulIdx < MEM_TYPE_MAX + 1; ulIdx++)
        {
            type_cnt[ulIdx] = OS_mem_list[ulIdx].cnt;
        }
    }

    if ((NULL != pctr_cnt))
    {
        *pctr_cnt = 0;
        for (ulIdx = 0; ulIdx < OS_MEM_CTRL_CNT; ulIdx++)
        {
            if (MEM_INIT == OS_mem_ctrl[ulIdx].state)
            {
                *pctr_cnt += 1;
            }
        }
    }
    OS_EXIT_MEM_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : os_mem_malloc
 Input        : ULONG32 len
 Output       : None
 Return Value : void*
 Description  : OS memory malloc function
 Calls        :

*****************************************************************************/
void* OS_mem_malloc(ULONG32 size)
{
    OS_MEM_TYPE type;
    OS_RET_CODE ret;
    OS_MEM_CTRL *plist;

#ifdef OS_DBG_MEM
    printf("\r\nmalloc %uB\r\n", size);
#endif
    OS_ENTER_MEM_CRITICAL();
    /* bigger than the supported maxsize */
    if (size > OS_mem_type[MEM_TYPE_MAX])
    {
        return NULL;
    }

    for (type = MEM_TYPE_MIN; type <= MEM_TYPE_MAX; type++)
    {
        if (OS_mem_type[type] >= size)
        {
            break;
        }
    }

    ret = __mem_del(type, type, &plist);
    if ((OS_RET_SUCCESS != ret) || (NULL == plist))
    {
        OS_EXIT_MEM_CRITICAL();
        return NULL;
    }
    plist->owner = OS_pCurrTask->id;
    OS_EXIT_MEM_CRITICAL();

    return (void*)(plist->addr);
}

/*****************************************************************************
 Prototype    : OS_mem_free
 Input        : void* pointer
 Output       : None
 Return Value : OS_RET_CODE
 Description  : OS memory free
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mem_free(void* pointer)
{
    OS_RET_CODE ret;
    OS_MEM_TYPE type;
    OS_MEM_CTRL *pprev, *preprev;
    OS_MEM_CTRL *plist;
    OS_MEM_CTRL *pnext, *pnextnext;
    ULONG32 addr;

#ifdef OS_DBG_MEM
    printf("\r\nfree 0x%X\r\n", pointer);
#endif

    if (NULL == pointer)
    {
        return OS_RET_PARAM;
    }

    addr = (ULONG32)pointer;

    OS_ENTER_MEM_CRITICAL();

    /* 查找对应的mem_ctrl */
    type = MEM_TYPE_MAX;
    while (1)
    {
        plist = OS_mem_list[type].head;
        if ((NULL != plist) && (addr >= plist->addr))
        {
            while (NULL != plist)
            {
                if ((addr == plist->addr) && (MEM_BUSY == plist->state))
                {
                    goto __found_addr;
                }
                plist = plist->next;
            }
        }

        if (MEM_TYPE_MIN == type--)
        {
            goto __not_found;
        }
    }

__not_found:

    OS_EXIT_MEM_CRITICAL();
    return OS_RET_NOT_FOUND;
__found_addr:

    /* 空列表或者最大的mem类型,直接释放 */
    if ((0 == OS_mem_list[type].cnt) || (MEM_TYPE_MAX == type))
    {
        goto __release_mem;
    }

    if (NULL != plist->prev)
    {
        pprev = plist->prev;
        /* 可以和前面的合并 */
        if ((MEM_FREE == pprev->state) && (pprev->addr + OS_mem_type[type] == addr))
        {
            pprev->state = MEM_INIT; /*释放这两个mem_ctrl*/
            plist->state = MEM_INIT;
            OS_mem_list[type].cnt -= 1;

            preprev = pprev->prev;
            pnextnext = plist->next;

            if (NULL == preprev)
            {
                OS_mem_list[type].head = pnextnext;
            }
            else
            {
                preprev->next = pnextnext;
            }

            if (NULL != pnextnext)
            {
                pnextnext->prev = preprev;
            }

            ret = __mem_add(__JOIN_TYPE(type), pprev->addr);
            OS_EXIT_MEM_CRITICAL();
            return ret;
        }
    }

    if (NULL != plist->next)
    {
        pnext = plist->next;
        /* 可以和后面的合并 */
        if ((MEM_FREE == pnext->state) && (OS_mem_type[type] + addr == pnext->addr))
        {
            pnext->state = MEM_INIT; /*释放这两个mem_ctrl*/
            plist->state = MEM_INIT;
            OS_mem_list[type].cnt -= 1;

            preprev = plist->prev;
            pnextnext = pnext->next;

            if (NULL == preprev)
            {
                OS_mem_list[type].head = pnextnext;
            }
            else
            {
                preprev->next = pnextnext;
            }

            if (NULL != pnextnext)
            {
                pnextnext->prev = preprev;
            }

            ret = __mem_add(__JOIN_TYPE(type), addr);
            OS_EXIT_MEM_CRITICAL();
            return ret;
        }
    }

__release_mem:
    plist->state = MEM_FREE;
    OS_mem_list[type].cnt++;

    OS_EXIT_MEM_CRITICAL();
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : OS_mem_malloc_stk
 Input        : ULONG size
                OS_TASK_ID task_id
 Output       : None
 Return Value : void*
 Description  : Malloc memory for a task
 Calls        :

*****************************************************************************/
void* OS_mem_malloc_stk(ULONG32 size, OS_TASK_ID task_id)
{
    OS_MEM_TYPE type;
    OS_RET_CODE ret;
    OS_MEM_CTRL *plist;

#ifdef OS_DBG_MEM
    printf("\r\nmalloc %uB\r\n", size);
#endif

    if (task_id >= OS_TASK_CNT_MAX)
    {
        return NULL;
    }

    OS_ENTER_MEM_CRITICAL();
    /* bigger than the supported maxsize */
    if (size > OS_mem_type[MEM_TYPE_MAX])
    {
        OS_EXIT_MEM_CRITICAL();
        return NULL;
    }

    for (type = MEM_TYPE_MIN; type <= MEM_TYPE_MAX; type++)
    {
        if (OS_mem_type[type] >= size)
        {
            break;
        }
    }

    ret = __mem_del(type, type, &plist);
    if ((OS_RET_SUCCESS != ret) || (NULL == plist))
    {
        OS_EXIT_MEM_CRITICAL();
        return NULL;
    }

    plist->owner = task_id;
    OS_EXIT_MEM_CRITICAL();

    return (void*)(plist->addr);
}

/*****************************************************************************
 Prototype    : OS_mem_free_stk
 Input        : OS_TASK_ID task_id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Free the task's stack
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mem_free_stk(OS_TASK_ID task_id)
{
    OS_RET_CODE ret;
    OS_MEM_TYPE type;
    OS_MEM_CTRL *pprev, *preprev;
    OS_MEM_CTRL *plist;
    OS_MEM_CTRL *pnext, *pnextnext;
    ULONG32 addr;

    addr = (ULONG32)(OS_TCBList[task_id].stk_bos);

#ifdef OS_DBG_MEM
    printf("\r\nfree 0x%X\r\n", pointer);
#endif

    /* 查找对应的mem_ctrl */
    type = MEM_TYPE_MAX;
    while (1)
    {
        plist = OS_mem_list[type].head;
        if ((NULL != plist) && (addr >= plist->addr))
        {
            while (NULL != plist)
            {
                if ((addr == plist->addr) && (MEM_BUSY == plist->state))
                {
                    goto __found_addr;
                }
                plist = plist->next;
            }
        }

        if (MEM_TYPE_MIN == type--)
        {
            goto __not_found;
        }
    }

__not_found:
    return OS_RET_NOT_FOUND;
__found_addr:

    /* 空列表或者最大的mem类型,直接释放 */
    if ((0 == OS_mem_list[type].cnt) || (MEM_TYPE_MAX == type))
    {
        goto __release_mem;
    }

    if (NULL != plist->prev)
    {
        pprev = plist->prev;
        /* 可以和前面的合并 */
        if ((MEM_FREE == pprev->state) && (pprev->addr + OS_mem_type[type] == addr))
        {
            pprev->state = MEM_INIT; /*释放这两个mem_ctrl*/
            plist->state = MEM_INIT;
            OS_mem_list[type].cnt -= 1;

            preprev = pprev->prev;
            pnextnext = plist->next;

            if (NULL == preprev)
            {
                OS_mem_list[type].head = pnextnext;
            }
            else
            {
                preprev->next = pnextnext;
            }

            if (NULL != pnextnext)
            {
                pnextnext->prev = preprev;
            }

            ret = __mem_add(__JOIN_TYPE(type), pprev->addr);
            return ret;
        }
    }

    if (NULL != plist->next)
    {
        pnext = plist->next;
        /* 可以和后面的合并 */
        if ((MEM_FREE == pnext->state) && (OS_mem_type[type] + addr == pnext->addr))
        {
            pnext->state = MEM_INIT; /*释放这两个mem_ctrl*/
            plist->state = MEM_INIT;
            OS_mem_list[type].cnt -= 1;

            preprev = plist->prev;
            pnextnext = pnext->next;

            if (NULL == preprev)
            {
                OS_mem_list[type].head = pnextnext;
            }
            else
            {
                preprev->next = pnextnext;
            }

            if (NULL != pnextnext)
            {
                pnextnext->prev = preprev;
            }

            ret = __mem_add(__JOIN_TYPE(type), addr);
            return ret;
        }
    }

__release_mem:
    plist->state = MEM_FREE;
    OS_mem_list[type].cnt++;
    return OS_RET_SUCCESS;
}


/*****************************************************************************
 Prototype    : OS_mem_release
 Input        : OS_TASK_ID task_id
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Release the mem a task has taken
 Calls        :

*****************************************************************************/
OS_RET_CODE OS_mem_release(OS_TASK_ID task_id)
{
    ULONG32 ulIdx;
    ULONG32 ulAddr;
    BOOL    freed = false;

    if (task_id >= OS_TASK_CNT_MAX)
    {
        return OS_RET_PARAM;
    }

    /* free all the memory this task has malloced */
    for (ulIdx = 0; ulIdx < OS_MEM_CTRL_CNT; ulIdx++)
    {
        OS_ENTER_MEM_CRITICAL();
        if ((task_id == OS_mem_ctrl[ulIdx].owner)
                && (OS_mem_ctrl[ulIdx].addr != (ULONG32)(OS_TCBList[task_id].stk_bos)))
        {
            ulAddr = OS_mem_ctrl[ulIdx].addr;
            OS_EXIT_MEM_CRITICAL();
            OS_mem_free((void*)ulAddr);
            freed = true;
        }
    }
    if (!freed)
    {
        OS_EXIT_MEM_CRITICAL();
    }
    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : __decrease_to_zero
 Input        : ULONG32 size
                OS_MEM_TYPE type
 Output       : None
 Return Value : BOOL
 Description  : Justice can the size be decreased to 0 from this type
 Calls        :

*****************************************************************************/
BOOL __decrease_to_zero(ULONG32 size, OS_MEM_TYPE type)
{
    size -= OS_mem_type[type];
    while (1)
    {
        if (size >= OS_mem_type[type])
        {
            size -= OS_mem_type[type];
        }
        else
        {
            if (0 == size)
            {
                return true;
            }
            if (MEM_TYPE_MIN == type--)
            {
                return false;
            }
        }
    }
}

/*****************************************************************************
 Prototype    : __mem_align
 Input        : ULONG32 size
                OS_MEM_TYPE type
 Output       : None
 Return Value : OS_MEM_TYPE
 Description  : Justice can this size be divided by this type
 Calls        :

*****************************************************************************/
OS_MEM_TYPE __mem_align(ULONG32 size)
{
    OS_MEM_TYPE type;

    type = MEM_TYPE_MAX;
    while (1)
    {
        if ((size < OS_mem_type[type]) || (!__decrease_to_zero(size, type)))
        {
            if (MEM_TYPE_MIN == type--)
            {
                return MEM_TYPE_INIT;
            }
        }
        else
        {
            return type;
        }
    }
}

/*****************************************************************************
 Prototype    : __mem_partiion
 Input        : void
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Repartition the memory pool
 Calls        :

*****************************************************************************/
OS_RET_CODE __mem_partiion(ULONG32 base_addr, ULONG32 size)
{
    UCHAR8 ret = OS_RET_SUCCESS;
    OS_MEM_TYPE type;

    type = __mem_align(size);
    if (MEM_TYPE_INIT == type)
    {
        return OS_RET_PARAM;
    }

    while (1)
    {
        if (size >= OS_mem_type[type])
        {
            ret = __mem_add(type, base_addr + size - OS_mem_type[type]);
            if (OS_RET_SUCCESS == ret)
            {
                size -= OS_mem_type[type];
                if (0 != size)
                {
                    continue;
                }
                else
                {
                    return OS_RET_SUCCESS;
                }
            }
            else
            {
                return OS_RET_FULL;
            }
        }

        if (MEM_TYPE_MIN == type--)
        {
            return OS_RET_PARAM;
        }
    }
}

/*****************************************************************************
 Prototype    : __mem_add
 Input        : OS_MEM_LIST list
                OS_MEM_ITEM *header
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Add memory to memlist
 Calls        :

*****************************************************************************/
OS_RET_CODE __mem_add(OS_MEM_TYPE type, ULONG32 base_addr)
{
    ULONG32 ulIdx;
    OS_MEM_CTRL *pcurr = OS_mem_list[type].head;
    OS_MEM_CTRL *pempty;
    OS_MEM_CTRL *pnext;

    /* 找到一个可用的mem_ctrl */
    for (ulIdx = 0; ulIdx < OS_MEM_CTRL_CNT; ulIdx++)
    {
        if (MEM_INIT == OS_mem_ctrl[ulIdx].state)
        {
            break;
        }
    }
    if (OS_MEM_CTRL_CNT == ulIdx)
    {
        return OS_RET_NO_MEM;
    }

    pempty = &OS_mem_ctrl[ulIdx];
    pempty->addr = base_addr;
    pempty->next = pempty->prev = NULL;
    pempty->type = type;

    if (NULL == pcurr)  /* 此类型的memlist为空,直接插入 */
    {
        pempty->state = MEM_FREE;
        OS_mem_list[type].head = pempty;
        OS_mem_list[type].cnt++;
        return OS_RET_SUCCESS;
    }

    /* plist != NULL */
    /* 找到mem_list内的恰当位置 */
    /* 比第一个节点小,放到头部 */
    if (base_addr < pcurr->addr)
    {
        /* 最大的mem type不合并 */
        if ((MEM_FREE == pcurr->state) && (base_addr+OS_mem_type[type] == pcurr->addr) && (MEM_TYPE_CNT-1 != type))
        {
            /* 和头部合并 */
            OS_mem_list[type].head = pcurr->next;
            if (NULL != pcurr->next)
            {
                pcurr->next->prev = NULL;
            }
            pcurr->type = MEM_TYPE_INIT;
            pcurr->state = MEM_INIT;
            OS_mem_list[type].cnt--;
            return __mem_add(__JOIN_TYPE(type), base_addr);
        }
        else
        {
            /* 放到头部 */
            pempty->state = MEM_FREE;
            pempty->prev = pcurr->prev;
            pcurr->prev = pempty;
            pempty->next = pcurr;
            OS_mem_list[type].head = pempty;
            OS_mem_list[type].cnt++;
            return OS_RET_SUCCESS;
        }

    }

    pnext = pcurr->next;
    while (pnext)
    {
        if ((pcurr->addr < base_addr) && (pnext->addr > base_addr))
        {
            break; /* find */
        }
        pcurr = pnext;
        pnext = pnext->next;
    }

    /* 放到尾部 */
    if (NULL == pnext)
    {
        /* 可以合并,最大的mem type不能合并 */
        if ((MEM_FREE == pcurr->state) && (pcurr->addr + OS_mem_type[type] == base_addr) && (MEM_TYPE_CNT-1 != type))
        {
            if (NULL != pcurr->prev)
            {
                pcurr->prev->next = NULL;
            }
            OS_mem_list[type].cnt--;
            pcurr->type = MEM_TYPE_INIT;    /* 释放mem_ctrl */
            pcurr->state = MEM_INIT;
            return __mem_add(__JOIN_TYPE(type), pcurr->addr);
        }
        else
        {   /* 不能合并,放到尾部  */
            pempty->state = MEM_FREE;
            pcurr->next = pempty;
            pempty->prev = pcurr;
            OS_mem_list[type].cnt++;
            return OS_RET_SUCCESS;
        }
    }

    /*(NULL != pnext)*/
    /* 放到中间 */

    if (MEM_TYPE_MAX != type)
    {
        /* 可以和前面的mem_ctrl合并 */
        if ((MEM_FREE == pcurr->state) && (pcurr->addr + OS_mem_type[type]== base_addr))
        {
            if (NULL == pcurr->prev) /* head */
            {
                OS_mem_list[type].head = pcurr->next;
                if (NULL != pcurr->next)
                {
                    pcurr->next->prev = NULL;
                }
            }
            else
            {
                pcurr->prev->next = pcurr->next;
                pcurr->next->prev = pcurr->prev;
            }
            OS_mem_list[type].cnt--;
            pcurr->type = MEM_TYPE_INIT;    /* 释放mem_ctrl */
            pcurr->state = MEM_INIT;
            return __mem_add(__JOIN_TYPE(type), pcurr->addr);
        }
        else if ((MEM_FREE == pnext->state) && (OS_mem_type[type] + base_addr == pnext->addr))
        {   /* 可以和后面的mem_ctrl合并 */
            {
                pcurr->next = pnext->next;
                if (NULL != pnext->next)
                {
                    pnext->next->prev = pcurr;
                }
            }
            OS_mem_list[type].cnt--;
            pnext->type = MEM_TYPE_INIT;    /* 释放mem_ctrl */
            pnext->state = MEM_INIT;
            return __mem_add(__JOIN_TYPE(type), base_addr);
        }
    }

    /* 不能合并,直接插入节点 */
    pempty->state = MEM_FREE;
    pcurr->next = pempty;
    pempty->prev = pcurr;
    pempty->next = pnext;
    pnext->prev = pempty;
    OS_mem_list[type].cnt++;

    return OS_RET_SUCCESS;
}

/*****************************************************************************
 Prototype    : __mem_del
 Input        : OS_MEM_LIST *plist
 Output       : None
 Return Value : void*
 Description  : Get a mem block
 Calls        :

*****************************************************************************/
OS_RET_CODE __mem_del(OS_MEM_TYPE ori_type, OS_MEM_TYPE type, OS_MEM_CTRL **pplist)
{
    OS_RET_CODE ret = OS_RET_SUCCESS;
    OS_MEM_CTRL *plist = OS_mem_list[type].head;
    OS_MEM_CTRL *pnext, *pprev;

    *pplist = NULL;
    if ((NULL == plist) || (0 == OS_mem_list[type].cnt))
    {
        if (MEM_TYPE_MAX == type)
        {
            return OS_RET_NO_MEM;
        }
        else
        {
            return __mem_del(ori_type, __NEXT_TYPE(type), pplist);
        }
    }

    while (NULL != plist)
    {
        if (MEM_FREE == plist->state)
        {
            goto __alloc_mem;
        }
        plist = plist->next;
    }
    return OS_RET_NO_MEM;

__alloc_mem:
    *pplist = plist;
    plist->state = MEM_BUSY;
    plist->type = type;
    OS_mem_list[type].cnt--;
    if (ori_type != type)
    {
        /* 如果剩余的内存能够成功划分就移动,否则不移动,防止出现碎片 */
        ret = __mem_partiion(plist->addr + OS_mem_type[ori_type],
                OS_mem_type[type] - OS_mem_type[ori_type]);
        if (OS_RET_SUCCESS == ret)
        {
            pprev = plist->prev;
            pnext = plist->next;

            ret = __mem_move(ori_type, plist);/* 移动到了其他类型的list */
            if (OS_RET_SUCCESS != ret)
            {
                return ret;
            }

            /* 把这个mem_ctrl从原mem_list删除 */
            if (NULL != pprev)
            {
                pprev->next = pnext;
            }
            else
            {
                OS_mem_list[type].head = pnext;
            }

            if (NULL != pnext)
            {
                pnext->prev = pprev;
            }
        }
        else
        {
            return OS_RET_SUCCESS; /* 即使没有了mem_ctrl也算分配成功了 */
        }
    }
    return ret;
}

/*****************************************************************************
 Prototype    : __mem_move
 Input        : OS_MEM_TYPE type
                OS_MEM_CTRL *plist
 Output       : None
 Return Value : OS_RET_CODE
 Description  : Move a mem_ctrl to new mem_list
 Calls        :

*****************************************************************************/
OS_RET_CODE __mem_move(OS_MEM_TYPE type, OS_MEM_CTRL *plist)
{
    OS_MEM_CTRL *pcurr = OS_mem_list[type].head;
    OS_MEM_CTRL *pnext;

    plist->type = type;

    if (NULL == pcurr)  /* 此类型的memlist为空,直接插入 */
    {
        plist->next = plist->prev = NULL;
        OS_mem_list[type].head = plist;
        return OS_RET_SUCCESS;
    }

    /* plist != NULL */
    /* 找到mem_list内的恰当位置 */
    /* 比第一个节点小,放到头部 */
    if (plist->addr < pcurr->addr)
    {
        /* 放到头部 */
        plist->prev = pcurr->prev;
        pcurr->prev = plist;
        plist->next = pcurr;
        OS_mem_list[type].head = plist;
        return OS_RET_SUCCESS;
    }

    pnext = pcurr->next;
    while (pnext)
    {
        if ((pcurr->addr < plist->addr) && (pnext->addr > plist->addr))
        {
            break; /* find */
        }
        pcurr = pnext;
        pnext = pnext->next;
    }

    /* 放到尾部 */
    if (NULL == pnext)
    {
        /* 不能合并,放到尾部  */
        pcurr->next = plist;
        plist->prev = pcurr;
        plist->next = NULL;
        return OS_RET_SUCCESS;
    }

    /*(NULL != pnext)*/
    /* 放到中间 */
    /* 不能合并,直接插入节点 */
    pcurr->next = plist;
    plist->prev = pcurr;
    plist->next = pnext;
    pnext->prev = plist;

    return OS_RET_SUCCESS;
}


/*****************************************************************************
 Prototype    : OS_mem_dbginfo
 Input        : void
 Output       : None
 Return Value : void
 Description  : Memory debug function
 Calls        :

*****************************************************************************/
void OS_mem_info(void)
{
    ULONG32 ulIdx;
#ifdef OS_DBG_MEM
    OS_MEM_CTRL *plist;
#endif
    ULONG32 ctrl_cnt;
    ULONG32 total_size;
    USHORT16 mem_type;
    ULONG32 mem_cnt;


    printf(PRINT_GAP);
    OS_mem_query(NULL, &ctrl_cnt);
    total_size = 0;
    for (ulIdx = 0; ulIdx < MEM_TYPE_CNT; ulIdx++ )
    {
        OS_ENTER_MEM_CRITICAL();
        mem_type = OS_mem_type[ulIdx];
        mem_cnt = OS_mem_list[ulIdx].cnt;
        OS_EXIT_MEM_CRITICAL();
        printf("%4dB---%d,", mem_type, mem_cnt);
#ifdef OS_DBG_MEM
        OS_ENTER_MEM_CRITICAL();
        plist = OS_mem_list[ulIdx].head;
        printf("head");
        while (NULL != plist)
        {
            printf("->(%X,%d)", plist->addr, plist->state);
            plist = plist->next;
        }
        OS_EXIT_MEM_CRITICAL();
#endif /* #ifdef OS_DBG_MEM */
        printf("\r\n");

        total_size += mem_cnt * mem_type;
    }
    printf("ctrl   :%u\r\n", ctrl_cnt);
    printf("memory :%dKB,%dB\r\n", total_size/1024, total_size%1024);
    printf(PRINT_GAP);
}
#endif /* #ifdef OS_SUPPORT_MEM */
