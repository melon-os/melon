/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_mailbox.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/23
    Last Modified :
    Description   : Melon OS mailbox header file

    History       :
        1.Date        : 2013/6/23
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#ifndef __OS_MAILBOX_H__
#define __OS_MAILBOX_H__

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
typedef enum tag_os_mbox_state {
    OS_MBOX_FREE = 0x0,     /* unused */
    OS_MBOX_BUSY,           /* allocated to one task */
} OS_MBOX_STATE;
typedef struct tag_os_mbox_msg {
    OS_MSG_ID           id;         /* msg id */
    OS_TASK_ID          dst;        /* destination task id */
    OS_TASK_ID          src;        /* source task id */
    OS_MSG_TIMER        tmo_timer;  /* timeout of this msg(not use yet!) */
    OS_MSG_DATA         content;    /* msg content, you can use it as the msg len */
    OS_MSG_PEXD         pexd;       /* pointer of msg extend data */
} OS_MBOX_MSG;

typedef struct tag_os_mbox {
    OS_MBOX_ID          id;         /* mailbox id */
    OS_MBOX_STATE       state;      /* mailbox state */
    OS_MSG_ID           msg_cnt;    /* msg counter of a mailbox */
    OS_MBOX_MSG         msg[OS_MBOX_MSG_CNT_MAX]; /* msg list in a mailbox */
    OS_TASK_ID          owner;      /* creator task of this mailbox */
} OS_MBOX;

typedef struct tag_os_mbox_pend_info {
    OS_TASK_STATE_EXD    type       :8; /* pend type, should be OS_TASK_PEND_MBOX */
    OS_TASK_STATE_EXD    ret        :8; /* pend ret code */
    OS_TASK_STATE_EXD    id         :16;/* mailbox id */
    OS_TASK_STATE_EXD    reserved   :16;
    OS_TASK_STATE_EXD    tick       :16;/* timeout of reveive msg */
} OS_MBOX_PEND_INFO;

typedef enum tag_os_mbox_pend_ret {
    OS_MBOX_PEND_INIT = 0x0,    /* initialization state */
    OS_MBOX_PEND_ING,           /* receiver is still pending */
    OS_MBOX_PEND_OK,            /* receiver gets msg before timeout */
    OS_MBOX_PEND_TIMEOUT,       /* receiver does not gets msg before timeout */
} OS_MBOX_PEND_RET;


/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
#define OS_MBOX_ID_INVALID          OS_MBOX_CNT_MAX
#define OS_MBOX_MSG_ID_MAX      OS_MBOX_MSG_CNT_MAX

#ifdef OS_MBOX_SUPPORT_INTER
#define  OS_ENTER_MBOX_CRITICAL() do {\
    cpu_sr = OS_int_disable();\
} while(0);
#define  OS_EXIT_MBOX_CRITICAL() do {\
    OS_int_enable(cpu_sr);\
} while(0);
#else
#define OS_ENTER_MBOX_CRITICAL() do {\
    OS_mutex_lock(OS_mbox_mutex_id, 0, 0);\
} while (0);
#define OS_EXIT_MBOX_CRITICAL() do {\
    OS_mutex_unlock(OS_mbox_mutex_id);\
} while (0);
#endif

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern OS_RET_CODE OS_mailbox_create(OS_MBOX_ID *pMboxId);
extern OS_RET_CODE OS_mailbox_delete(OS_MBOX_ID mbox_id, OS_TASK_TIMER tmo_tick);
extern OS_RET_CODE OS_mailbox_release(OS_TASK_ID task_id, OS_TASK_TIMER tmo_tick);
extern OS_RET_CODE OS_mailbox_init(void);
extern OS_RET_CODE OS_mailbox_post(OS_MBOX_ID mbox_id, OS_MBOX_MSG *pMsg, OS_MSG_OPT opt);
extern OS_RET_CODE OS_mailbox_recv(OS_MBOX_ID mbox_id , OS_MBOX_MSG *pMsg, OS_MSG_TIMER tmo_timer);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __OS_MAILBOX_H__ */
