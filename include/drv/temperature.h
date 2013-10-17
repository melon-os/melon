#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_
#include <common_func.h>

extern UCHAR8 Temperature_get_temp(UINT16 *pulValue);
extern void Temperaturn_init_18b20 (void);

#endif /*_TEMPERATURE_H_*/
