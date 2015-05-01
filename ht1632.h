#ifndef HT1632_H
#define HT1632_H

#include <inttypes.h>
#include "pins.h"

#define HT1632_MODE_READ			0x06
#define HT1632_MODE_WRITE			0x05
#define HT1632_MODE_COMMAND			0x04

#define HT1632_CMD_SYS_OFF			0x00
#define HT1632_CMD_SYS_ON			0x01
#define HT1632_CMD_LED_OFF			0x02
#define HT1632_CMD_LED_ON			0x03
#define HT1632_CMD_BLINK_OFF		0x08
#define HT1632_CMD_BLINK_ON			0x09
#define HT1632_CMD_SLAVE			0x10
#define HT1632_CMD_MASTER			0x18
#define HT1632_CMD_MASTER_EXT		0x1C
#define HT1632_CMD_COM_NMOS_08		0x20
#define HT1632_CMD_COM_NMOS_16		0x24
#define HT1632_CMD_COM_PMOS_08		0x28
#define HT1632_CMD_COM_PMOS_16		0x2C
#define HT1632_CMD_DUTY				0xA0

#define HT1632_DUTY_MAX				0x0F

void ht1632SendCmd(uint8_t cmd);
void ht1632Init(void);
void ht1632SetAddr(uint8_t addr);
void ht1632SendDataBuf(uint8_t *buf);


#endif /* HT1632_H */
