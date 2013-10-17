#ifndef _CLOCK_H_
#define _CLOCK_H_
#include <common_func.h>


/*从1970年开始计数*/
#define CLOCK_TIME_ZERO_YEAR    (1970)
/* (year mod 4 = 0 and year mod 100 != 0) Or (year mod 400 = 0) */
#define __leep_year(year) (((0 == year%4) && (0 != year%100)) || (0 == year%400))

/*
一个字节BCD码转换为 DEC码 
例如BCD 0x32 转换为DEC为 32

*/
#if 1
#define BCD_2_DEC(data) ( \
            (((data) & 0xF0) << 1)\
            + (((data) & 0xF0) << 3)\
            + ((data) & 0x0F))
#define DEC_2_BCD(data) (\
            (((data) / 10) << 4) \
            + ((data) % 10))
#else
#define BCD_2_DEC(data) (data/16*10 + data%16)
#define DEC_2_BCD(data) (data/10*16 + data%10)
#endif
typedef enum {
    TIME_ITEM_SECOND = 0x0,
    TIME_ITEM_MINUTE,
    TIME_ITEM_HOUR,
    TIME_ITEM_DAY,
    TIME_ITEM_MONTH,
    TIME_ITEM_WEEK,
    TIME_ITEM_YEAR,
    TIME_ITEM_CNT,
} TIME_ITEM_E;

typedef struct tag_CLOCK_TIME {
    UCHAR8 ucSecond;
    UCHAR8 ucMinute;
    UCHAR8 ucHour;
    UCHAR8 ucDay;
    UCHAR8 ucMonth;
    UCHAR8 ucWeek;
    USHORT16 usYear;
} CLOCK_TIME_S;

/******************************************************************/
/*                    函数声明                                    */
/******************************************************************/
extern void Clock_get_time(CLOCK_TIME_S *pstTime);
extern void Clock_set_time(CLOCK_TIME_S *pstTime);
extern UCHAR8 Clock_set_time_ui(void);
extern void Clock_time_demo(void *p_args);
extern ULONG32 Clock_mktime(CLOCK_TIME_S * tm);
extern ULONG32 Clock_get_clock(void);
#endif /* #ifndef _CLOCK_H_ */
