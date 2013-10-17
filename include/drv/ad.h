#ifndef _AD_H_
#define _AD_H_
#include <common_func.h>


#define PCF8591 0x90    //PCF8591 µÿ÷∑
#define AD_TEMP_CH_ADDR 0x42
#define AD_PR_CH_ADDR   0x43

/* power voltage is 3.3 V */
#define VDD                 3300
/* 1.43v at 25C */
#define V25_CLASSIC         1430
/* 4.3mv/C */
#define AVG_SLOPE           4.3
#define ADC1_DR_Address    ((u32)0x4001244C)

extern UCHAR8 AD_Init(void);
extern UCHAR8 AD_get_pr(USHORT16 *pusPrData);
extern UCHAR8 AD_get_temp(USHORT16 *pusTemp2);
extern UCHAR8 AD_get_temp_mcu_core(USHORT16 *pusADCValue);

#endif/*#ifndef _AD_H_*/
