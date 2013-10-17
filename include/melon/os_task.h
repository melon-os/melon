/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_task.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS task header file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/

#ifndef __OS_TASK_H__
#define __OS_TASK_H__
#include <melon/melon.h>
#include <melon/os_mutex.h>
#include <melon/os_mailbox.h>

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
typedef enum tag_os_task_state {
    OS_TASK_UNINIT = 0x0,
    OS_TASK_INIT,           /* init state */
    OS_TASK_READY,          /* ready to go */
    OS_TASK_RUN,            /* running, only 1 task is running at the same time */
    OS_TASK_PEND,           /* wait for timer, mutex or mailbox */
} OS_TASK_STATE;

typedef enum tag_os_task_option {
    OS_TASK_OPT_NONE = 0,
    OS_TASK_OPT_USER = 1<<0, /* user level task */
    OS_TASK_OPT_SYSTEM = 1<<1,  /* system level task */
    OS_TASK_OPT_UNDELETED = 1<<2, /* cann't be deleted by other tasks */
    OS_TASK_OPT_DIV = 1<<3, /* task supports division operation */
    OS_TASK_OPT_STATIC = 1<<4, /* task stack is static memory */
    OS_TASK_OPT_MALLOC = 1<<5, /* task stack is malloc memory */
} OS_TASK_OPTION;

typedef struct tag_os_timer_pend_info {
    OS_TASK_STATE_EXD    type :8;
    OS_TASK_STATE_EXD    ret  :8;
    OS_TASK_STATE_EXD    reserved  :16;
    OS_TASK_STATE_EXD    tick :32;
} OS_TIMER_PEND_INFO;

typedef enum tasg_os_task_pend_flag {
    OS_TASK_PEND_MUTEX = 1,
    OS_TASK_PEND_MBOX = 2,
    OS_TASK_PEND_TIMER = 3,
} OS_TASK_PEND_FLAG;

typedef struct tag_os_tcb {
    OS_STK                  *stk_tos;       /* top of stack */
    OS_STK_SIZE             stk_size;
    OS_STK                  *stk_bos;   /* bottom of stack, const */
    OS_TASK_ID              id;
    OS_TASK_STATE           state;
    union {
        OS_TASK_STATE_EXD   whole;
        OS_TIMER_PEND_INFO  timer;
        OS_MUTEX_PEND_INFO  mutex;
        OS_MBOX_PEND_INFO   mbox;
    } pend;
    OS_TASK_PRIORITY        priority;
    OS_TASK_PRIORITY        original_priority;
    OS_TASK_TIMER           active_tick; /* 一个调度周期内剩余的生命点数 */
#ifdef OS_SUPPORT_STAT
    OS_TASK_TIMER           switch_counter; /* 成功切换到运行的tick数目 */
#endif /*#ifdef OS_SUPPORT_STAT*/
    OS_TASK_OPT             opt;
    OS_TASK_NAME            name;
} OS_TCB;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern OS_RET_CODE OS_task_init(void);
extern OS_RET_CODE OS_task_create(void (*task)(void *pParam), void *pParam,
                                      OS_TASK_PRIORITY priority,
                                      OS_STK *pstk_top, OS_STK_SIZE size,
                                       OS_TASK_NAME name, OS_TASK_OPT opt,
                                        OS_TASK_ID *id);
extern OS_TASK_ID OS_task_gettid(void);
extern OS_TASK_STATE OS_task_query_id(OS_TASK_ID task_id);
extern OS_TASK_STATE OS_task_query_name(CHAR8 *pname);
extern OS_RET_CODE OS_task_delete(OS_TASK_ID task_id);
extern void OS_task_exit(void);
extern OS_RET_CODE OS_task_sleep(ULONG32 ultimer);
extern OS_RET_CODE OS_task_tcb_init( OS_TASK_PRIORITY priority,
                                       OS_STK *stk_tos,
                                        OS_STK_SIZE size,
                                         OS_TASK_NAME name,
                                          OS_TASK_OPT opt,
                                           OS_TASK_ID *id);
extern void OS_task_idle(void *p_arg);

#ifdef OS_SUPPORT_STAT
extern void OS_task_stat( void *p_arg );
extern OS_TASK_ID OS_task_get_stat(OS_TASK_ID aId[OS_TASK_CNT_MAX],
                                     OS_TASK_TIMER aUseage[OS_TASK_CNT_MAX],
                                      OS_STK_SIZE aStk_size[OS_TASK_CNT_MAX],
                                      OS_STK_SIZE aStk_curr_size[OS_TASK_CNT_MAX],
                                      OS_TASK_NAME aName[OS_TASK_CNT_MAX],
                                       OS_TASK_PRIORITY apri[OS_TASK_CNT_MAX],
                                        OS_TASK_TIMER aActive_tick[OS_TASK_CNT_MAX],
                                         OS_SCHD_PERIOD *pPeriod, OS_SCHD_PERIOD *pCycle,
                                          OS_SCHD_PERIOD *pCtxStwCnt);
#endif /*#ifdef OS_SUPPORT_STAT*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __OS_TASK_H__ */
