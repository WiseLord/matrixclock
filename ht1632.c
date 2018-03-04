#include "ht1632.h"

#include <avr/pgmspace.h>
#include "pins.h"

static void ht1632SendBits(uint8_t cnt, uint8_t bits)
{
    int8_t i;

    for (i = cnt - 1; i >= 0; i--) {
        if (bits & (1 << i))
            SET(HT1632_DATA);
        else
            CLR(HT1632_DATA);

        CLR(HT1632_WR);
        asm("nop");
        asm("nop");
        SET(HT1632_WR);
    }
}

void ht1632SendCmd(uint8_t cmd)
{
    CLR(HT1632_CS);
    ht1632SendBits(HT1632_IDBITS_CNT, HT1632_MODE_COMMAND);
    ht1632SendBits(HT1632_CMDBITS_CNT, cmd);
    ht1632SendBits(HT1632_CMDLASTBIT, 0);
    SET(HT1632_CS);
}

void ht1632SendDataBuf(uint8_t *buf)
{
    uint8_t i;

    CLR(HT1632_CS);
    ht1632SendBits(HT1632_IDBITS_CNT, HT1632_MODE_WRITE);
    ht1632SendBits(HT1632_ADDRBITS_CNT, 0);
    for (i = 0; i < MATRIX_CNT * 8; i++)
        ht1632SendBits(HT1632_DATABITS_CNT, buf[i]);
    SET(HT1632_CS);
}

void ht1632Init(void)
{
    OUT(HT1632_DATA);
    OUT(HT1632_CS);
    OUT(HT1632_WR);

    SET(HT1632_DATA);
    SET(HT1632_CS);
    SET(HT1632_WR);

    ht1632SendCmd(HT1632_CMD_SYS_ON);
    ht1632SendCmd(HT1632_CMD_LED_ON);
    ht1632SendCmd(HT1632_CMD_BLINK_OFF);
    ht1632SendCmd(HT1632_CMD_MASTER);
    ht1632SendCmd(HT1632_CMD_COM_NMOS_08);
    ht1632SendCmd(HT1632_CMD_DUTY);
}
