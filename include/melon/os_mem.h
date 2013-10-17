/******************************************************************************
    Copyright (C), 2004-2013, melon_os@163.com, All Rights Reserved

    * LICENSING TERMS:
    * ---------------
    * Melon is provided in source form for FREE evaluation, for educational use or for peaceful research.
    * If you plan on using Melon in a commercial product you need to contact me to properly license
    * its use in your product. I provide ALL the source code for your convenience and to help you experience
    * Melon.  The fact that the source is provided does NOT mean that you can use it without paying a
    * licensing fee.

    File Name     : os_mem.h
    Version       : Initial Draft
    Author        : QQ 381624054
    Created       : 2013/6/30
    Last Modified :
    Description   : Melon OS memory management header file

    History       :
        1.Date      : 2013/6/30
        Author      : QQ 381624054
        Modification: Created file
******************************************************************************/
#ifndef __OS_MEM_H__
#define __OS_MEM_H__

//#define OS_DBG_MEM

/*----------------------------------------------*
 * internal routine prototypes                  *
 *----------------------------------------------*/
 typedef enum tag_os_mem_type {
    MEM_8B = 0x0,
    MEM_12B,
    MEM_16B,
    MEM_24B,
    MEM_32B,
    MEM_48B,
    MEM_64B,
    MEM_96B,
    MEM_128B,
    MEM_192B,
    MEM_256B,
    MEM_384B,
    MEM_512B,
    MEM_768B,
    MEM_1024B,
    MEM_1536B,
    MEM_2048B,
    MEM_3072B,
    MEM_4096B,
    MEM_TYPE_CNT,
    MEM_TYPE_MIN = MEM_8B,
    MEM_TYPE_MAX = MEM_4096B,
    MEM_TYPE_INIT,
} OS_MEM_TYPE;

typedef enum tag_os_mem_ctrl_state {
    MEM_INIT = 0x0,
    MEM_FREE,
    MEM_BUSY,
} OS_MEM_CTRL_STATE;

typedef struct tag_os_mem_ctrl {
    struct tag_os_mem_ctrl     *prev;
    struct tag_os_mem_ctrl     *next;
    ULONG32             addr;
    OS_MEM_TYPE         type;
    OS_MEM_CTRL_STATE   state;
    OS_TASK_ID          owner;
} OS_MEM_CTRL;

typedef struct tag_os_mem_list {
    OS_MEM_CTRL        *head;
    ULONG32             cnt;
} OS_MEM_LIST;

/*----------------------------------------------*
 * macros                                       *
 *----------------------------------------------*/
extern OS_MUTEX_ID OS_mem_mutex_id;
#define OS_ENTER_MEM_CRITICAL() do {\
    OS_mutex_lock(OS_mem_mutex_id, 0, 0);\
} while (0);
#define OS_EXIT_MEM_CRITICAL() do {\
    OS_mutex_unlock(OS_mem_mutex_id);\
} while (0);

#define __JOIN_TYPE(type) ((OS_MEM_TYPE)(type+2))
#define __NEXT_TYPE(type) ((OS_MEM_TYPE)(type+1))

/*----------------------------------------------*
 * routines' implementations                    *
 *----------------------------------------------*/


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern OS_RET_CODE OS_mem_init(void);
extern OS_RET_CODE OS_mem_query(ULONG32 type_cnt[MEM_TYPE_CNT], ULONG32 *pctr_cnt);
extern void* OS_mem_malloc(ULONG32 len);
extern OS_RET_CODE OS_mem_free(void* pointer);
extern void* OS_mem_malloc_stk(ULONG32 size, OS_TASK_ID task_id);
extern OS_RET_CODE OS_mem_free_stk(OS_TASK_ID task_id);
extern OS_RET_CODE OS_mem_release(OS_TASK_ID task_id);
extern void OS_mem_info(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __OS_MEM_H__ */

