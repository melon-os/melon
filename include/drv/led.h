#ifndef _LED_H_
#define _LED_H_

#include <common_func.h>


#define LED_LEN_X               0x7
#define LED_LEN_Y               0x7
#define DELAY_TIMER             10
#define DELAY_DISP_POINT        20

/* 每个字符做8次扫描 */
#define NUM_SCAN_TIMES		    8
#define NUM_DISP_GAP_MS         1
#define NUM_DISP_TIME           2000

typedef enum {
    RED=0x0,    
    YELLOW,
    GREEN,
    COLOR_CNT,
} COLOR_E;


typedef enum {
    FONT_0 = 0x0,
    FONT_1,
    FONT_CNT,
    FONT_MAX = FONT_1
} FONE_MODE_E;

extern void add_led(unsigned char x, unsigned char y, unsigned color);
extern void light_led(unsigned char x, unsigned char y, 
                unsigned char len_x, unsigned char len_y, unsigned color);

extern void led_init(void);
extern UCHAR8 led_change_font_mode(FONE_MODE_E ucFontMode);

extern void led_display_num(UCHAR8 num, COLOR_E color, unsigned long display_time);

extern void led_drew_rect_demo(void);
extern void led_display_num_demo(void);
extern void led_disp_num_rand_color_demo(void);


#endif /*#ifndef _LED_H_*/
