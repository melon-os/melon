/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : melon.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/12
    Last Modified :
    Description   : Melon OS header file

    History       :
        1.Date      : 2013/6/12
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/

#ifndef __MELON_H__
#define __MELON_H__
#include <arch/os_cpu.h>
#include <melon/melon_cfg.h>

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#ifndef false
#define false   (0)
#endif
#ifndef true
#define true    (!false)
#endif
#ifndef EOC
#define EOC     ('\0')
#endif
#ifndef NULL
#define NULL    ((void*)0)
#endif

#ifdef MELON_DBG
#define __AFFIRM(exp) do {\
    if (!(exp))\
    {\
        while (1);\
    }\
} while (0);
#else
#define __AFFIRM(exp)
#endif /* #ifdef MELON_DBG */

#define PRINT_GAP   "---------------------------------------------------------\n\r"
#define OS_LOGO \
"-------------------------------------------------\r\n"\
"--mel-------onm----------------------------------\r\n"\
"--melo-----nmel----------------------------------\r\n"\
"--melon---melon--------- Melon OS          ------\r\n"\
"--mel-on-me-lon--------- Ver "OS_VERSION"         ------\r\n"\
"--mel--onm--elo--------- melon_os@163.com  ------\r\n"\
"--mel-------onm----------------------------------\r\n"\
"-------------------------------------------------\r\n"

#define OS_GET_STK_TOP(arr) ((OS_STK *)(&arr[(sizeof(arr)/sizeof(arr[0]))-1]))
#define OS_GET_STK_SIZE(arr) (sizeof(arr))
#define OS_GET_ARRAY_CNT(arr) (sizeof(arr)/sizeof(arr[0]))
#define OS_HIGHER_PRIORITY(pria, prib) (((pria) > (prib)) ? (prib) : (pria))
#define OS_TASK_WEIGHT(pri) (((OS_TASK_PRIORITY_LOWEST - (pri)) >> 4) + 1)
#define  OS_ENTER_CRITICAL() do {\
    cpu_sr = OS_int_disable(); \
} while(0);
#define  OS_EXIT_CRITICAL() do {\
    OS_int_enable(cpu_sr);\
} while(0);


/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/
typedef ULONG32     OS_SCHD_PERIOD;
/* task members's type definition */
typedef USHORT16    OS_TASK_ID;
typedef ULONG32     OS_STK;
typedef USHORT16    OS_STK_SIZE;
typedef UCHAR8      OS_TASK_PRIORITY;
typedef CHAR8       OS_TASK_NAME[OS_TASK_NAME_LEN_MAX];
typedef ULONG64     OS_TASK_STATE_EXD;
typedef ULONG32     OS_TASK_TIMER;
typedef USHORT16    OS_TASK_OPT;
/* mutex members's type definition */
typedef USHORT16    OS_MUTEX_ID;
typedef CHAR8       OS_MUTEX_NAME[OS_MUTEX_NAME_LEN_MAX];
typedef USHORT16    OS_MUTEX_TIMER;
typedef USHORT16    OS_MUTEX_OPT;
/* mailbox members's type definition */
typedef USHORT16    OS_MBOX_ID;
typedef USHORT16    OS_MSG_ID;
typedef USHORT16    OS_MSG_OPT;
typedef USHORT16    OS_MSG_TIMER;
typedef ULONG32     OS_MSG_DATA;
typedef void*       OS_MSG_PEXD;
/* timer members's type definition */
typedef USHORT16 OS_TIMER_ID;
typedef ULONG32 OS_TIMER_PERIOD;
typedef void (*OS_TIMER_CALLBACK)(void);
typedef CHAR8 OS_TIMER_NAME[OS_TIMER_NAME_LEN_MAX];
/* interrupt type definition */
typedef ULONG32     OS_ISR_STATE;

/* OS return code enumeration */
typedef enum tag_os_return_code {
    OS_RET_SUCCESS = 0x0,
    OS_RET_FAIL,
    OS_RET_PARAM,
    OS_RET_UNINIT,
    OS_RET_BUSY,
    OS_RET_TIMEOUT,
    OS_RET_NO_MEM,
    OS_RET_EMPTY,
    OS_RET_FULL,
    OS_RET_OVERFLOW,
    OS_RET_UNDERFLOW,
    OS_RET_NOT_READY,
    OS_RET_NOT_FOUND,
    OS_RET_NOT_SUPPORT,
} OS_RET_CODE;

typedef enum tag_os_state {
    OS_STATE_INIT = 0x0,
    OS_STATE_RUN,
    OS_STATE_SCHED_LOCK, /* schedule lock */
} OS_STATE;

typedef struct tag_os_info {
    OS_STATE            state;
    OS_TASK_ID          task_cnt;
    OS_SCHD_PERIOD      sched_period;   /* num of ticks in one schedule cycle */
    OS_SCHD_PERIOD      sched_counter;    /* num of ticks left in one schedule cycle */
    OS_SCHD_PERIOD      ctx_swt_cnt;   /* context switch counter */
} OS_INFO;

#endif /* __MELON_H__ */
