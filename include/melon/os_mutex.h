/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_mutex.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/23
    Last Modified :
    Description   : Melon OS mutex header file

    History       :
        1.Date      : 2013/6/23
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/

#ifndef __OS_MUTEX_H__
#define __OS_MUTEX_H__

#include <melon/melon.h>

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
 typedef enum tag_os_mutex_state {
    OS_MUTEX_UNINIT = 0x0,   /* unused state */
    OS_MUTEX_INIT,           /* in init process */
    OS_MUTEX_UNLOCK,         /* can be locked by task */
    OS_MUTEX_LOCKED,         /* already locked by task */
}OS_MUTEX_STATE;

typedef struct tag_os_mutex {
    OS_MUTEX_ID     id;
    OS_MUTEX_STATE  state;
    OS_TASK_ID      owner;                                 /* which task has taken this mutex */
    OS_TASK_ID      wait_taskcnt;                              /* task number, which is waiting for this mutex */
#ifdef OS_MUTEX_SUPPORT_NAME
    OS_MUTEX_NAME   name;
#endif
    OS_MUTEX_OPT    flag;
} OS_MUTEX;

typedef struct tag_os_mutex_pend_info {
    OS_TASK_STATE_EXD    type       :8;
    OS_TASK_STATE_EXD    ret        :8;
    OS_TASK_STATE_EXD    id         :16;
    OS_TASK_STATE_EXD    tick       :16;
    OS_TASK_STATE_EXD    reserved   :16;
} OS_MUTEX_PEND_INFO;

typedef enum tag_os_mutex_pend_ret {
    OS_MUTEX_PEND_INIT = 0x0,
    OS_MUTEX_PEND_ING,  /* still pending */
    OS_MUTEX_PEND_OK,   /* get the mutex before timeout */
    OS_MUTEX_PEND_TIMEOUT,
} OS_MUTEX_PEND_RET;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
extern OS_RET_CODE OS_mutex_init(void);
extern OS_RET_CODE OS_mutex_create(OS_MUTEX_NAME name, OS_MUTEX_OPT opt, OS_MUTEX_ID *pMutex_id);
extern OS_RET_CODE OS_mutex_trylock(OS_MUTEX_ID mutex_id);
extern OS_RET_CODE OS_mutex_lock(OS_MUTEX_ID mutex_id, OS_MUTEX_TIMER timeout, OS_MUTEX_OPT opt);
extern OS_RET_CODE OS_mutex_unlock(OS_MUTEX_ID mutex_id);
extern OS_RET_CODE OS_mutex_release(OS_TASK_ID task_id);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __OS_MUTEX_H__ */
