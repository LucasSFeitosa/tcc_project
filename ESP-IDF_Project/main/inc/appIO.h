#ifndef APP_IO_H_
#define APP_IO_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C"
{
#endif

void    IO_peripheralsInit(void);
uint8_t IO_relayToggle(void);
uint8_t IO_setStateRelay(uint8_t state);
uint8_t IO_getStateRelay(void);
uint8_t IO_getStateLevelSensor(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_CONFIG_H_ */