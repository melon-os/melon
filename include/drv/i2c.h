#ifndef _I2C_H
#define _I2C_H
#include <common_func.h>


extern bit I2C_ack_bitflag;
//起动总线函数
extern void Start_I2c(void);
//结束总线函数  
extern void Stop_I2c(void);
//应答子函数
extern void Ack_I2c(bit a);
//字节数据发送函数
extern void  SendByte(UCHAR8  c);
//有子地址发送多字节数据函数               
extern bit ISendStr(UCHAR8 sla,UCHAR8 suba,UCHAR8 *s,UCHAR8 no) ;
//无子地址发送多字节数据函数   
extern bit ISendStrExt(UCHAR8 sla,UCHAR8 *s,UCHAR8 no);
//无子地址读字节数据函数               
extern UCHAR8 RcvByte(void);

#endif/*#ifndef _I2C_H*/
            
