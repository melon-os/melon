#include <drv/clock.h>
#include <drv/ad.h>
#include <drv/temperature.h>
#include <drv/lcd.h>


#define MINUTE 60
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define YEAR (365*DAY)

/* interestingly, we assume leap-years */
static ULONG32 month[12] = {
	0,
	DAY*(31),
	DAY*(31+29),
	DAY*(31+29+31),
	DAY*(31+29+31+30),
	DAY*(31+29+31+30+31),
	DAY*(31+29+31+30+31+30),
	DAY*(31+29+31+30+31+30+31),
	DAY*(31+29+31+30+31+30+31+31),
	DAY*(31+29+31+30+31+30+31+31+30),
	DAY*(31+29+31+30+31+30+31+31+30+31),
	DAY*(31+29+31+30+31+30+31+31+30+31+30)
};

//秒分时日月周年 最低位读写位
code UCHAR8 write_rtc_address[TIME_ITEM_CNT] = {0x80,0x82,0x84,0x86,0x88,0x8a,0x8c};
code UCHAR8 read_rtc_address[TIME_ITEM_CNT] = {0x81,0x83,0x85,0x87,0x89,0x8b,0x8d};

#define RECT_HEIGHT     50
#define RECT_WIDTH      240
LCD_RECT_S idata stDate_rect = {
    0,
    0,
    0,
    RECT_HEIGHT,
    RECT_WIDTH,
    red,
    white,
    {0},
    LCD_RECT_REPAINT_ALL,
    NULL,
};

LCD_RECT_S idata stTime_rect = {
    1,
    0,
    134,
    RECT_HEIGHT,
    RECT_WIDTH,
    white,
    blue,
    {0},
    LCD_RECT_REPAINT_ALL,
    NULL,
};

LCD_RECT_S idata stTemp_rect = {
    2,
    0,
    267,
    RECT_HEIGHT,
    RECT_WIDTH,
    blue,
    white,
    {0},
    LCD_RECT_REPAINT_ALL,
    NULL,
};


LCD_RECT_S idata stMove_rect = {
    0xFF,
    0,
    0,
    32,
    32,
    white,
    black,
    {0x0},
    LCD_RECT_REPAINT_ALL,
    NULL,
};

static void Write_Ds1302_Byte(unsigned  char temp);
static void Write_Ds1302(UCHAR8 address,UCHAR8 dat);
static UCHAR8 Read_Ds1302 (UCHAR8 address);
static void Read_RTC(UCHAR8 ucLocal_buf[TIME_ITEM_CNT]);//read RTC
static void Set_RTC(UCHAR8 ucLocal_buf[TIME_ITEM_CNT]); //set RTC
//static void Rect_rand_move_demo(void);

/******************************************************************/
/*                   主函数                                       */
/******************************************************************/
void Clock_get_time(CLOCK_TIME_S *pstTime)
{
    data UCHAR8 ucYear = 0;

    if (NULL == pstTime)
    {
        return;
    }

    Read_RTC((UCHAR8 *)pstTime);

    pstTime->ucSecond = BCD_2_DEC(pstTime->ucSecond);
    pstTime->ucMinute = BCD_2_DEC(pstTime->ucMinute);
    pstTime->ucHour = BCD_2_DEC(pstTime->ucHour);
    pstTime->ucDay = BCD_2_DEC(pstTime->ucDay);
    pstTime->ucMonth = BCD_2_DEC(pstTime->ucMonth);
    pstTime->ucWeek = BCD_2_DEC(pstTime->ucWeek);
#if defined(_CPU_C51_STC90C516RD_)
    /* c51 is big endian*/
    ucYear = pstTime->usYear  >> 8;
#elif defined(_CPU_ARM_STM32F103C8_)
    ucYear = pstTime->usYear;
#else
#error "INVALID CPU TYPE"
#endif
    pstTime->usYear = BCD_2_DEC(ucYear);
    pstTime->usYear += CLOCK_TIME_ZERO_YEAR;

}

void Clock_set_time(CLOCK_TIME_S *pstTime)
{
    data UCHAR8 ucYear = 0;

    if (NULL == pstTime)
    {
        return;
    }

    /* DEC to BCD*/
    pstTime->ucSecond = DEC_2_BCD(pstTime->ucSecond);
    pstTime->ucMinute = DEC_2_BCD(pstTime->ucMinute);
    pstTime->ucHour = DEC_2_BCD(pstTime->ucHour);
    pstTime->ucDay = DEC_2_BCD(pstTime->ucDay);
    pstTime->ucMonth = DEC_2_BCD(pstTime->ucMonth);
    pstTime->ucWeek = DEC_2_BCD(pstTime->ucWeek);

    pstTime->usYear -= CLOCK_TIME_ZERO_YEAR;
    ucYear = pstTime->usYear;
    /* c51 is big endian*/
    ucYear = DEC_2_BCD(ucYear);
#if defined(_CPU_C51_STC90C516RD_)
    pstTime->usYear = ucYear << 8;
#elif defined(_CPU_ARM_STM32F103C8_)
    pstTime->usYear = ucYear;
#else
#error "INVALID CPU TYPE"
#endif

    Set_RTC((UCHAR8 *)pstTime);

}
#if 1
UCHAR8 Clock_set_time_ui(void)
{
    ULONG32 ulInput0=0, ulInput1=0, ulInput2=0;
    data CLOCK_TIME_S tm = {0};

    Clock_get_time(&tm);
    Sys_printf("Input month day week\n");
    scanf("%d%d%d", &ulInput0, &ulInput1, &ulInput2);
    tm.ucMonth = ulInput0;
    tm.ucDay = ulInput1;
    tm.ucWeek = ulInput2;
    Sys_printf("Input hour min sec\n");
    scanf("%d%d%d", &ulInput0, &ulInput1, &ulInput2);
    tm.ucHour = ulInput0;
    tm.ucMinute = ulInput1;
    tm.ucSecond = ulInput2;

    Clock_set_time(&tm);
    Sys_printf("\nNew time:\n%d-%d-%d (%d)\n%d:%d:%d\n",
        (ULONG32)tm.usYear,
        (ULONG32)tm.ucMonth,
        (ULONG32)tm.ucDay,
        (ULONG32)tm.ucWeek,
        (ULONG32)tm.ucHour,
        (ULONG32)tm.ucMinute,
        (ULONG32)tm.ucSecond
    );

    return ERR_CODE_OK;
}
#endif

/******************************************************************/
/*                   写一个字节                                   */
/******************************************************************/
void Write_Ds1302_Byte(unsigned  char temp)
{
    UCHAR8 i;
    for (i=0;i<8;i++)     	//循环8次 写入数据
    {
        //CLK_SCK = 0;
        GPIO_RESET_BIT(GPIOA, CLK_SCK);
        /*CLK_SDA=temp & 0x01; 每次传输低字节 */
        if (temp & 0x01)
        {
           // GPIO_WRITE(GPIOA, CLK_SDA, );
           GPIO_SET_BIT(GPIOA, CLK_SDA);
        }
        else
        {
            GPIO_RESET_BIT(GPIOA, CLK_SDA);
        }

        temp >>= 1;  		//右移一位
        _nop_();
        /*CLK_SCK = 1;*/
        GPIO_SET_BIT(GPIOA, CLK_SCK);
    }
}
/******************************************************************/
/*                  写入DS1302                                    */
/******************************************************************/
void Write_Ds1302( UCHAR8 address,UCHAR8 dat )
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* CLK 的DATA端口 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;    //所有GPIO为同一类型端口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //输出的最大频率为50HZ
    GPIO_Init(GPIOA, &GPIO_InitStructure);   //初始化GPIOA端口



    /*CLK_RST=0;*/
    GPIO_RESET_BIT(GPIOA, CLK_RST);
	_nop_();

 	/*CLK_SCK=0;*/
    GPIO_RESET_BIT(GPIOA, CLK_SCK);

    _nop_();
 	/*CLK_RST=1;*/
    GPIO_SET_BIT(GPIOA, CLK_RST);
   	_nop_();    /*                //启动*/

 	Write_Ds1302_Byte(address);	//发送地址
 	Write_Ds1302_Byte(dat);		//发送数据
 	/*CLK_RST=0;  		          恢复*/
    _nop_();
 	GPIO_RESET_BIT(GPIOA, CLK_RST);

    GPIO_SET_BIT(GPIOA, CLK_SDA);
}
/******************************************************************/
/*                   读出DS1302数据                               */
/******************************************************************/
UCHAR8 Read_Ds1302 ( UCHAR8 address )
{
 	UCHAR8 i,temp=0x00;

    GPIO_InitTypeDef GPIO_InitStructure;
    /* CLK 的DATA端口 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;    //所有GPIO为同一类型端口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //输出的最大频率为50HZ
    GPIO_Init(GPIOA, &GPIO_InitStructure);   //初始化GPIOA端口



 	/*CLK_RST=0;*/
    GPIO_RESET_BIT(GPIOA, CLK_RST);
    _nop_();
    _nop_();
 	//CLK_SCK=0;
    GPIO_RESET_BIT(GPIOA, CLK_SCK);
	//_nop_();
	//_nop_();
	_nop_();
    _nop_();
 	//CLK_RST=1;
    GPIO_SET_BIT(GPIOA, CLK_RST);
	_nop_();
	_nop_();
 	Write_Ds1302_Byte(address);

    /* CLK 的DATA端口 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;    //所有GPIO为同一类型端口
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     //输出的最大频率为50HZ
    GPIO_Init(GPIOA, &GPIO_InitStructure);   //初始化GPIOA端口

 	for (i=0;i<8;i++) 		//循环8次 读取数据
 	{
 		//if(CLK_SDA)
 		if (1 == GPIO_GET_BIT(GPIOA, CLK_SDA))
 		temp|=0x80;			//每次传输低字节
		//CLK_SCK=0;
        GPIO_RESET_BIT(GPIOA, CLK_SCK);
        //GPIO_SET_BIT(GPIOA, CLK_SCK);
		temp>>=1;			//右移一位
		_nop_();
	    //_nop_();
	    //_nop_();
 		//CLK_SCK=1;
        GPIO_SET_BIT(GPIOA, CLK_SCK);
        //GPIO_RESET_BIT(GPIOA, CLK_SCK);
        //_nop_();
	}
// 	CLK_RST=0;

    _nop_();
    GPIO_RESET_BIT(GPIOA, CLK_RST);
#if 0

    _nop_();//以下为DS1302复位的稳定时间
 	_nop_();
	//CLK_RST=0;
	GPIO_RESET_BIT(GPIOA, CLK_RST);

//	CLK_SCK=0;
    GPIO_RESET_BIT(GPIOA, CLK_SCK);
    _nop_();
    _nop_();
    _nop_();
    _nop_();

	//CLK_SCK=1;
	GPIO_SET_BIT(GPIOA, CLK_SCK);
    _nop_();
    _nop_();

	//CLK_SDA=0;
    GPIO_RESET_BIT(GPIOA, CLK_SDA);
    _nop_();
    _nop_();
#endif


	//CLK_SDA=1;
    GPIO_SET_BIT(GPIOA, CLK_SDA);
    _nop_();
    _nop_();


	return (temp);			//返回
}
/******************************************************************/
/*                   读时钟数据                                   */
/******************************************************************/
void Read_RTC(UCHAR8 ucLocal_buf[TIME_ITEM_CNT])	        //读取 日历
{
    UCHAR8 i,*p;
    p = read_rtc_address;//地址传递
    for(i=0; i<TIME_ITEM_CNT; i++)//分7次读取 秒分时日月周年
    {
        ucLocal_buf[i] = Read_Ds1302(*p);
        p++;
    }
}
/******************************************************************/
/*                  设定时钟数据                                  */
/******************************************************************/
void Set_RTC(UCHAR8 ucLocal_buf[TIME_ITEM_CNT])		    //设定 日历
{
	UCHAR8 i,*p;

 	Write_Ds1302(0x8E, 0X00);
 	p = write_rtc_address;//传地址
 	for(i = 0; i < TIME_ITEM_CNT; i++)//7次写入 秒分时日月周年
 	{
// 	    ucLocal_buf[i] = DEC_2_BCD(ucLocal_buf[i]);
        Write_Ds1302(*p, ucLocal_buf[i]);
        p++;
	}
	Write_Ds1302(0x8E, 0x80);
}

ULONG32 Clock_get_clock(void)
{
    data CLOCK_TIME_S tm = {0};

    Clock_get_time(&tm);

    return Clock_mktime(&tm);
}
ULONG32 Clock_mktime(CLOCK_TIME_S * tm)
{
	data ULONG32 res;
	data USHORT16 year;

	year = tm->usYear - CLOCK_TIME_ZERO_YEAR;
/* magic offsets (y+1) needed to get leapyears right.*/
	res = YEAR*year + DAY*((year+1)/4);
	res += month[tm->ucMonth];
/* and (y+2) here. If it wasn't a leap-year, we have to adjust */
	if (tm->ucMonth>1 && ((year+2)%4))
		res -= DAY;
	res += DAY*(tm->ucDay-1);
	res += HOUR*tm->ucHour;
	res += MINUTE*tm->ucMinute;
	res += tm->ucSecond;
	return res;
}

UCHAR8 days_12m[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void next_sec(CLOCK_TIME_S *local_time)
{
    if (60 == ++local_time->ucSecond)
    {
        local_time->ucSecond = 0;
        if (60 == ++local_time->ucMinute)
        {
            local_time->ucMinute = 0;
            LCD_clear_screen();
            if (24 == ++local_time->ucHour)
            {
                local_time->ucHour = 0;
                local_time->ucWeek = (local_time->ucWeek + 1) % 7;
                if (2 == local_time->ucMonth)
                {
                    days_12m[1] = (__leep_year(local_time->usYear)) ? 29 : 28;
                }
                if (days_12m[local_time->ucMonth] == ++local_time->ucDay)
                {
                    local_time->ucDay = 1;
                    if (13 == ++local_time->ucMonth)
                    {
                        local_time->ucMonth = 1;
                        local_time->usYear++;
                    }
                }
            }
        }
    }
}

UCHAR8 aucDate[21] = {0};
UCHAR8 aucTime[16] = {0};
UCHAR8 aucTemp[24] = {0};
UCHAR8 aucClock[12] = {0};
UCHAR8 aucTemp2[48] = {0};
UCHAR8 aucLight[16] = {0};

void Clock_time_demo(void *p_args)
{
    CLOCK_TIME_S local_time = {0, 0, 0, 1, 1, 1, 2013};
    UINT16 ulSecond, ulMinute, ulHour, ulDay, ulMonth, ulWeek, ulYear;
    UINT16 uiBkDay = 0;
    UINT16 uiBkSecond = 0;
    ULONG32 ulCurrentTemp = 0;
    ULONG32 ulBkTemp = 0;
    ULONG32 ulClock = 0;
    ULONG32 ulbkClock = 0;
    USHORT16 usRandNum = 0;
    USHORT16 usPrValue = 0;
    USHORT16 usPrValueBk = 0;
    ULONG32 ulTemp2 = 0;
    ULONG32 ulTemp2Bk = 0;
    ULONG32 ulLocalSeed = 0;
    (void)p_args;


    LCD_clear_screen();
    ulLocalSeed = Clock_get_clock();
    srand(ulLocalSeed);
    //Sys_printf("\n\r\n\r\n\r\n\r\n\rSet seed:0x%08X\n", ulLocalSeed);

#if 0
    Clock_set_time(&local_time);
#endif

    Clock_get_time(&local_time);
    do {

        Temperature_get_temp((USHORT16*)&ulCurrentTemp);
        AD_get_temp((USHORT16*)&ulTemp2);
        AD_get_pr(&usPrValue);

        ulSecond = local_time.ucSecond;
        ulMinute = local_time.ucMinute;
        ulHour = local_time.ucHour;
        ulDay = local_time.ucDay;
        ulMonth = local_time.ucMonth;
        ulWeek = local_time.ucWeek;
        ulYear = local_time.usYear;

        ulClock = Clock_mktime(&local_time);

        /*数据更新*/
        if (uiBkDay != ulDay)
        {
            uiBkDay = ulDay;
            sprintf((char*)aucDate, "Date:%d-%02d-%02d (%d)", ulYear, ulMonth, ulDay, ulWeek);
            LCD_rect_set_content(&stDate_rect, aucDate);
        }

        if (ulbkClock != ulClock)
        {
            ulbkClock = ulClock;
             sprintf((char*)aucClock, "0x%08X", ulClock);
            LCD_rect_set_exdata(&stDate_rect, aucClock);
        }

        if (uiBkSecond != ulSecond)
        {
            uiBkSecond = ulSecond;
            sprintf((char*)aucTime, "Time:%02d:%02d:%02d", ulHour, ulMinute, ulSecond);
            LCD_rect_set_content(&stTime_rect, aucTime);
        }

        if (usPrValueBk != usPrValue)
        {
            usPrValueBk = usPrValue;
            sprintf((char*)aucLight, "Light:%03d", usPrValue);
            LCD_rect_set_exdata(&stTime_rect, aucLight);
        }

        if (ulBkTemp != ulCurrentTemp)
        {
            ulBkTemp = ulCurrentTemp;
            /* 转化成 double是因为 此处必须8字节对齐 */
            sprintf((char*)aucTemp, "TEMP1:%d.%02d C(%d)", ulCurrentTemp>>4,
              (ULONG32)((double)((ulCurrentTemp&0xF)/16.0)*100), ulCurrentTemp);
            LCD_rect_set_content(&stTemp_rect, aucTemp);
        }


        if (ulTemp2Bk != ulTemp2)
        {
           ulTemp2Bk = ulTemp2;

           sprintf((char*)aucTemp2, "TEMP2:%d.%02d C(%d)", ulTemp2/10,
             (ULONG32)(((double)ulTemp2/10.0 - (ulTemp2/10))
           *100), ulTemp2);
           LCD_rect_set_exdata(&stTemp_rect, aucTemp2);
        }


#if 1   /*图形变换*/
        switch (ulSecond)
        {
            case 0:
                usRandNum = rand();
                LCD_rect_set_bg_color(&stDate_rect, usRandNum);
                LCD_rect_set_content_color(&stDate_rect, ~usRandNum);
                break;
            case 10:
                usRandNum = rand();
                LCD_rect_set_bg_color(&stTime_rect, usRandNum);
                LCD_rect_set_content_color(&stTime_rect, ~usRandNum);
                break;
            case 20:
                usRandNum = rand();
                LCD_rect_set_bg_color(&stTemp_rect, usRandNum);
                LCD_rect_set_content_color(&stTemp_rect, ~usRandNum);
                break;
            case 30:
                usRandNum = rand()%(133 - RECT_HEIGHT)
                    + ((USHORT16)(stDate_rect.pos_y/133))*133;
                LCD_rect_reLocate(&stDate_rect, 0, usRandNum);
                break;
            case 35:
                usRandNum = rand()%(133 - RECT_HEIGHT)
                    + ((USHORT16)(stTime_rect.pos_y/133))*133;
                LCD_rect_reLocate(&stTime_rect, 0, usRandNum);
                break;
            case 40:
                usRandNum = rand()%(133 - RECT_HEIGHT)
                    + ((USHORT16)(stTemp_rect.pos_y/133))*133;
                LCD_rect_reLocate(&stTemp_rect, 0, usRandNum);
                break;
            case 25:
                LCD_drew_line(0, 133, LCD_SIZE_W-1, 133, black);
                LCD_drew_line(0, 266, LCD_SIZE_W-1, 266, black);
                break;
            case 5:
            case 15:
                usRandNum = rand();
                LCD_drew_line(0, 133, LCD_SIZE_W-1, 133, (PRT_COLOR_E)usRandNum);
                usRandNum = rand();
                LCD_drew_line(0, 266, LCD_SIZE_W-1, 266, (PRT_COLOR_E)usRandNum);
                break;
            case 45:
            case 50:
            case 55:
                LCD_rect_swap_pos(&stDate_rect, &stTemp_rect);
                LCD_rect_swap_pos(&stTemp_rect, &stTime_rect);
                break;
        }
#endif

        LCD_rect_drew(&stDate_rect);
        LCD_rect_drew(&stTime_rect);
        LCD_rect_drew(&stTemp_rect);

        OS_task_sleep(1000);
        next_sec(&local_time);
    } while(1);

}

#if 0
void Rect_rand_move_demo(void)
{
#define MOVE_ONCE_TIME  10

   static UCHAR8 ucDirect = D_315;
    UCHAR8 ucRet = 0;


    if ((stMove_rect.pos_x < MOVE_ONCE_TIME)
            || ((stMove_rect.pos_x + MOVE_ONCE_TIME) > LCD_SIZE_W)
            || (stMove_rect.pos_y < MOVE_ONCE_TIME)
            || ((stMove_rect.pos_y + MOVE_ONCE_TIME) > LCD_SIZE_H))
    {
REDO_TURN:
        ucDirect = rand() % D_CNT;
        LCD_rect_set_bg_color(&stMove_rect, rand()&0xFFFF);

    }
    switch (ucDirect)
    {
        case D_UP:
            ucRet = LCD_rect_reLocate(&stMove_rect, stMove_rect.pos_x, stMove_rect.pos_y - MOVE_ONCE_TIME);
            break;
        case D_DOWN:
            ucRet = LCD_rect_reLocate(&stMove_rect, stMove_rect.pos_x, stMove_rect.pos_y + MOVE_ONCE_TIME);
            break;
        case D_LEFT:
            ucRet = LCD_rect_reLocate(&stMove_rect, stMove_rect.pos_x - MOVE_ONCE_TIME, stMove_rect.pos_y);
            break;
        case D_RIGHT:
            ucRet = LCD_rect_reLocate(&stMove_rect, stMove_rect.pos_x + MOVE_ONCE_TIME, stMove_rect.pos_y);
            break;
        case D_45:
            ucRet = LCD_rect_reLocate(&stMove_rect,
                stMove_rect.pos_x + rand()%MOVE_ONCE_TIME,
                stMove_rect.pos_y - rand()%MOVE_ONCE_TIME);
            break;
        case D_135:
            ucRet = LCD_rect_reLocate(&stMove_rect,
                stMove_rect.pos_x - rand()%MOVE_ONCE_TIME,
                stMove_rect.pos_y - rand()%MOVE_ONCE_TIME);

            break;
        case D_225:
            ucRet = LCD_rect_reLocate(&stMove_rect,
                stMove_rect.pos_x - rand()%MOVE_ONCE_TIME,
                stMove_rect.pos_y + rand()%MOVE_ONCE_TIME);

            break;
        case D_315:
            ucRet = LCD_rect_reLocate(&stMove_rect,
                stMove_rect.pos_x + rand()%MOVE_ONCE_TIME,
                stMove_rect.pos_y + rand()%MOVE_ONCE_TIME);

            break;
    }

    if (ERR_CODE_OK != ucRet)
    {
        goto REDO_TURN;
    }
}
#endif

#if 0
/*
        AD_get_temp_mcu_core(&usMcuTemp);
        dMcuTemp = (V25_CLASSIC - ((usMcuTemp * VDD) / 0xFFF))/AVG_SLOPE + 25;
        Vtemp_sensor = ADC_ConvertedValue * Vdd / Vdd_convert_value;
        Current_Temp = (V25 - Vtemp_sensor)/Avg_Slope + 25;
*/
#endif

