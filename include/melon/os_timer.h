/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_timer.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013-7-31
    Last Modified :
    Description   : Melon OS timer header file

    History       :
        1.Date      : 2013-7-31
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/

#ifndef __OS_TIMER_H__
#define __OS_TIMER_H__
#include <melon/melon.h>

#ifdef OS_SUPPORT_TIMER
/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef enum tag_os_timer_state {
    OS_TIMER_INIT = 0x0,
    OS_TIMER_STOP,
    OS_TIMER_RUN,
    OS_TIMER_PAUSE
} OS_TIMER_STATE;

typedef struct tag_os_timer {
    OS_TIMER_ID         id;
    OS_TIMER_STATE      state;
    OS_TIMER_PERIOD     period;
    OS_TIMER_PERIOD     original_period;
    OS_TIMER_PERIOD     times;
    OS_TIMER_CALLBACK   callback;
#ifdef OS_TIMER_SUPPORT_NAME
    OS_TIMER_NAME       name;
#endif
} OS_TIMER;

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define OS_ENTER_TIMER_CRITICAL() do {\
    OS_mutex_lock(OS_timer_mutex_id, 0, 0);\
} while (0);
#define OS_EXIT_TIMER_CRITICAL() do {\
    OS_mutex_unlock(OS_timer_mutex_id);\
} while (0);

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
extern OS_RET_CODE OS_timer_create(OS_TIMER_NAME name, OS_TIMER_PERIOD period,
                                                 OS_TIMER_PERIOD times, OS_TIMER_CALLBACK callback,
                                                     OS_TIMER_ID *tid);
extern OS_RET_CODE OS_timer_delete(OS_TIMER_ID tid);
extern OS_RET_CODE OS_timer_init(void);
extern OS_RET_CODE OS_timer_pause(OS_TIMER_ID tid);
extern OS_RET_CODE OS_timer_resume(OS_TIMER_ID tid);
extern OS_RET_CODE OS_timer_start(OS_TIMER_ID tid);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /* #ifdef OS_SUPPORT_TIMER */

#endif /* __OS_TIMER_H__ */
