 
/*************************�˲���Ϊ18B20����������*************************************/

#include <common_func.h>

extern void Sys_printf(unsigned char *fmt, ...);
void TempDelay (unsigned char idata us);
void Init18b20 (void);
void WriteByte (unsigned char idata wr);  //���ֽ�д��
void read_bytes (unsigned char idata j);
unsigned char Temp_check_crc (unsigned char j);
void GemTemp (void);
//void Temperaturn_init_18b20 (void);
void ReadID (void);

unsigned int  idata Temperature = 0;
unsigned char idata temp_buff[9]; //�洢��ȡ���ֽڣ�read scratchpadΪ9�ֽڣ�read rom IDΪ8�ֽ�
unsigned char idata id_buff[8];
unsigned char idata *p,TIM;
unsigned char idata crc_data;


const unsigned char CrcTable [256]={
0,  94, 188,  226,  97,  63,  221,  131,  194,  156,  126,  32,  163,  253,  31,  65,
157,  195,  33,  127,  252,  162,  64,  30,  95,  1,  227,  189,  62,  96,  130,  220,
35,  125,  159,  193,  66,  28,  254,  160,  225,  191,  93,  3,  128,  222,  60,  98,
190,  224,  2,  92,  223,  129,  99,  61,  124,  34,  192,  158,  29,  67,  161,  255,
70,  24,  250,  164,  39,  121,  155,  197,  132,  218,  56,  102,  229,  187,  89,  7,
219,  133, 103,  57,  186,  228,  6,  88,  25,  71,  165,  251,  120,  38,  196,  154,
101,  59, 217,  135,  4,  90,  184,  230,  167,  249,  27,  69,  198,  152,  122,  36,
248,  166, 68,  26,  153,  199,  37,  123,  58,  100,  134,  216,  91,  5,  231,  185,
140,  210, 48,  110,  237,  179,  81,  15,  78,  16,  242,  172,  47,  113,  147,  205,
17,  79,  173,  243,  112,  46,  204,  146,  211,  141,  111,  49,  178,  236,  14,  80,
175,  241, 19,  77,  206,  144,  114,  44,  109,  51,  209,  143,  12,  82,  176,  238,
50,  108,  142,  208,  83,  13,  239,  177,  240,  174,  76,  18,  145,  207,  45,  115,
202,  148, 118,  40,  171,  245,  23,  73,  8,  86,  180,  234,  105,  55,  213, 139,
87,  9,  235,  181,  54,  104,  138,  212,  149,  203,  41,  119,  244,  170,  72,  22,
233,  183,  85,  11,  136,  214,  52,  106,  43,  117,  151,  201,  74,  20,  246,  168,
116,  42,  200,  150,  21,  75,  169,  247,  182,  232,  10,  84,  215,  137,  107,  53}; 
//
/************************************************************
*Function:��ʱ����
*parameter:
*Return:
*Modify:
*************************************************************/
void TempDelay (unsigned char idata us)
{
#if defined(OSC_MODE_24M)
    us *= 5;
#endif
	while(us--);
}


/************************************************************
*Function:18B20��ʼ��
*parameter:
*Return:
*Modify:
*************************************************************/
void Init18b20 (void)
{
    GPIO_SET_BIT(GPIOA, D18B20);
	_nop_();
    GPIO_RESET_BIT(GPIOA, D18B20);

    Sys_delay_cpu_us(530);//delay 530 uS//80

    GPIO_SET_BIT(GPIOA, D18B20);

    Sys_delay_cpu_us(100); //delay 100 uS//14

	if (0 != GPIO_GET_BIT(GPIOA, D18B20))
	{ 
        Sys_printf("temper init fail!\n\r");
    }
    
	Sys_delay_cpu_us(132); //132 us
	_nop_();
	_nop_();
	GPIO_SET_BIT(GPIOA, D18B20);
}

/************************************************************
*Function:��18B20д��һ���ֽ�
*parameter:
*Return:
*Modify:
*************************************************************/
void WriteByte (unsigned char idata wr)  //���ֽ�д��
{
	unsigned char idata i;
	for (i=0;i<8;i++)
	{
		//D18B20 = 0;
        GPIO_RESET_BIT(GPIOA, D18B20);
		_nop_();
		//D18B20=wr&0x01;
        GPIO_WRITE(GPIOA, D18B20, wr&0x01);
		
		Sys_delay_cpu_us(50);//delay 50 uS
		//D18B20=1;
        GPIO_SET_BIT(GPIOA, D18B20);
		wr >>= 1;
        _nop_();
        _nop_();
        _nop_();
	}
    _nop_();
    _nop_();
    _nop_();
}

/************************************************************
*Function:��18B20��һ���ֽ�
*parameter:
*Return:
*Modify:
*************************************************************/
unsigned char ReadByte (void)     //��ȡ���ֽ�
{ 
	unsigned char idata i,u=0;
	for(i=0;i<8;i++)
	{
		//D18B20 = 0;
        GPIO_RESET_BIT(GPIOA, D18B20);
		u >>= 1;
        Sys_delay_cpu_us(1);
		//D18B20 = 1;
        GPIO_SET_BIT(GPIOA, D18B20);
		//if(D18B20==1)
		Sys_delay_cpu_us(1);
		if (1 == GPIO_GET_BIT(GPIOA, D18B20))
		    u |= 0x80;
		
        Sys_delay_cpu_us(15);
        _nop_();
	}
    _nop_();
    _nop_();
    _nop_();
	return(u);
}

/************************************************************
*Function:��18B20
*parameter:
*Return:
*Modify:
*************************************************************/
void read_bytes (unsigned char idata j)
{
	 unsigned char idata i;
	 for(i=0;i<j;i++)
	 {
		  *p = ReadByte();
		  p++;
	 }
}

/************************************************************
*Function:CRCУ��
*parameter:
*Return:
*Modify:
*************************************************************/
unsigned char Temp_check_crc (unsigned char j)
{
   	unsigned char idata i,crc_data=0;
  	for(i=0;i<j;i++)  //���У��
    	crc_data = CrcTable[crc_data^temp_buff[i]];
    return (crc_data);
}

/************************************************************
*Function:��ȡ�¶�
*parameter:
*Return:
*Modify:
*************************************************************/
void GemTemp (void)
{

   read_bytes (9);
   if (Temp_check_crc(9)==0) //У����ȷ
   {
	    Temperature = temp_buff[1]*0x100 + temp_buff[0];
   }
   else
   {
        Temperature = temp_buff[1]*0x100 + temp_buff[0];
        Sys_printf("crc check error!(%d)\n\r", Temperature);
   }   
}

/************************************************************
*Function:�ڲ�����
*parameter:
*Return:
*Modify:
*************************************************************/
void Temperaturn_init_18b20 (void)  //�������ñ����޶�ֵ�ͷֱ���
{
     Init18b20();
     WriteByte(0xcc);  //skip rom
     WriteByte(0x4e);  //write scratchpad
     WriteByte(0x19);  //����
     WriteByte(0x1a);  //����
     WriteByte(0x7f);     //set 12 bit (0.125)
     Init18b20();
     WriteByte(0xcc);  //skip rom
     WriteByte(0x48);  //�����趨ֵ
     Init18b20();
     WriteByte(0xcc);  //skip rom
     WriteByte(0xb8);  //�ص��趨ֵ
}

/************************************************************
*Function:��18B20ID
*parameter:
*Return:
*Modify:
*************************************************************/
void ReadID (void)//��ȡ���� id
{
	Init18b20();
	WriteByte(0x33);  //read rom
	read_bytes(8);
}

/************************************************************
*Function:18B20IDȫ����
*parameter:
*Return:
*Modify:
*************************************************************/
unsigned char Temperature_get_temp(UINT16 *pulValue)
{
#if 0
    *pulValue = 0xff;
    return ERR_CODE_OK;
#else    
  	p = id_buff;
	Init18b20 ();
	WriteByte(0xcc);   //skip rom
	WriteByte(0x44);   //Temperature convert

	Init18b20 ();
	WriteByte(0xcc);   //skip rom
	WriteByte(0xbe);   //read Temperature
	p = temp_buff;
	GemTemp();
    *pulValue = Temperature;
    return ERR_CODE_OK;
#endif
}
