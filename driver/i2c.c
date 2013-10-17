
/*************************此部分为I2C总线的驱动程序*************************************/

#include <drv/i2c.h>

bit I2C_ack_bitflag;                 /*应答标志位*/   

/*******************************************************************
                     起动总线函数               
函数原型: void  Start_I2c();  
功能:     启动I2C总线,即发送I2C起始条件.  
********************************************************************/
void Start_I2c(void)
{
  //I2C_SDA=1;         /*发送起始条件的数据信号*/
  GPIO_SET_BIT(GPIOB, I2C_SDA);
  _nop_();
  //I2C_SCLK=1;
  GPIO_SET_BIT(GPIOB, I2C_SCLK);
  _nop_();        /*起始条件建立时间大于4.7us,延时*/
  _nop_();
  _nop_();     
  //I2C_SDA=0;         /*发送起始信号*/
  GPIO_RESET_BIT(GPIOB, I2C_SDA);
  _nop_();        /* 起始条件锁定时间大于4μs*/
  _nop_();
  _nop_();        
  //I2C_SCLK=0;       /*钳住I2C总线，准备发送或接收数据 */
  GPIO_RESET_BIT(GPIOB, I2C_SCLK);
  _nop_(); 
}

/*******************************************************************
                      结束总线函数               
函数原型: void  Stop_I2c();  
功能:     结束I2C总线,即发送I2C结束条件.  
********************************************************************/
void Stop_I2c(void)
{
  //I2C_SDA=0;      /*发送结束条件的数据信号*/
  GPIO_RESET_BIT(GPIOB, I2C_SDA);
  _nop_();       /*发送结束条件的时钟信号*/
  //I2C_SCLK=1;      /*结束条件建立时间大于4μs*/
  GPIO_SET_BIT(GPIOB, I2C_SCLK);
  _nop_();
  _nop_();
  _nop_(); 
 // I2C_SDA=1;      /*发送I2C总线结束信号*/
  GPIO_SET_BIT(GPIOB, I2C_SDA);
  _nop_();
  _nop_();  
}

/*******************************************************************
                 字节数据发送函数               
函数原型: void  SendByte(UCHAR c);
功能:     将数据c发送出去,可以是地址,也可以是数据,发完后等待应答,并对
          此状态位进行操作.(不应答或非应答都使ack=0)     
           发送数据正常，ack=1; I2C_ack_bitflag=0表示被控器无应答或损坏。
********************************************************************/
void  SendByte(UCHAR8  c)
{
    UCHAR8  BitCnt;

    for(BitCnt=0;BitCnt<8;BitCnt++)  /*要传送的数据长度为8位*/
    {
        if((c<<BitCnt)&0x80)   /*判断发送位*/
        {  
            //I2C_SDA=1;
            GPIO_SET_BIT(GPIOB, I2C_SDA);
        }
        else
        {
           // I2C_SDA=0;
            GPIO_RESET_BIT(GPIOB, I2C_SDA);
        }
        _nop_();
        //I2C_SCLK=1;               /*置时钟线为高，通知被控器开始接收数据位*/
        GPIO_SET_BIT(GPIOB, I2C_SCLK);
        _nop_(); 
        _nop_();             /*保证时钟高电平周期大于4μs*/
        _nop_();            
       // I2C_SCLK=0; 
        GPIO_RESET_BIT(GPIOB, I2C_SCLK);
    }
    
    _nop_();
    //I2C_SDA=1;                /*8位发送完后释放数据线，准备接收应答位*/
    GPIO_SET_BIT(GPIOB, I2C_SDA);
    _nop_(); 
    //I2C_SCLK=1;
    GPIO_SET_BIT(GPIOB, I2C_SCLK);
    _nop_();
    _nop_();
    //if(I2C_SDA==1)/*判断是否接收到应答信号*/
    if (1 == GPIO_GET_BIT(GPIOB, I2C_SDA))
    {
        I2C_ack_bitflag=0;  
    }
    else
    {
        I2C_ack_bitflag=1;
    }
    //I2C_SCLK=0;
    GPIO_RESET_BIT(GPIOB, I2C_SCLK);
    _nop_();
    _nop_();
}

/*******************************************************************
                 字节数据接收函数               
函数原型: UCHAR  RcvByte();
功能:        用来接收从器件传来的数据,并判断总线错误(不发应答信号)，
          发完后请用应答函数应答从机。  
********************************************************************/    
UCHAR8   RcvByte(void)
{
    UCHAR8  retc;
    UCHAR8  BitCnt;

    retc=0; 
    //I2C_SDA=1;                     /*置数据线为输入方式*/
    GPIO_SET_BIT(GPIOB, I2C_SDA);
    for(BitCnt=0;BitCnt<8;BitCnt++)
    {
        _nop_();           
        //I2C_SCLK=0;                  /*置时钟线为低，准备接收数据位*/
        GPIO_RESET_BIT(GPIOB, I2C_SCLK);
        _nop_();
        _nop_();                 /*时钟低电平周期大于4.7μs*/
        _nop_();
        //I2C_SCLK=1;                  /*置时钟线为高使数据线上数据有效*/
        GPIO_SET_BIT(GPIOB, I2C_SCLK);

        _nop_();
        retc=retc<<1;
        //if(I2C_SDA==1)
        if (1 == GPIO_GET_BIT(GPIOB, I2C_SDA))
        {
        retc=retc+1;  /*读数据位,接收的数据位放入retc中 */
        }
        _nop_();
    }
    //I2C_SCLK=0;
    GPIO_RESET_BIT(GPIOB, I2C_SCLK);
    _nop_();
    return(retc);
}

/********************************************************************
                     应答子函数
函数原型:  void Ack_I2c(bit a);
功能:      主控器进行应答信号(可以是应答或非应答信号，由位参数a决定)
********************************************************************/
void Ack_I2c(bit a)
{

    if(a==0)
    { 
        //I2C_SDA=0;              /*在此发出应答或非应答信号 */
        GPIO_RESET_BIT(GPIOB, I2C_SDA);
    }
    else
    {
        //I2C_SDA=1;
        GPIO_SET_BIT(GPIOB, I2C_SDA);
    }
    _nop_();
    _nop_();    
    //I2C_SCLK=1;
    GPIO_SET_BIT(GPIOB, I2C_SCLK);
    _nop_();
    _nop_();                    /*时钟低电平周期大于4μs*/
   
    //I2C_SCLK=0;                     /*清时钟线，钳住I2C总线以便继续接收*/
    GPIO_RESET_BIT(GPIOB, I2C_SCLK);

    _nop_();
}
