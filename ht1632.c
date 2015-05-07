#include "ht1632.h"

#include <avr/pgmspace.h>

static void ht1632SendBits(uint8_t cnt, uint8_t bits)
{
	int8_t i;

	for (i = cnt - 1; i >= 0; i--) {
		if (bits & (1<<i))
			PORT(HT1632_DATA) |= HT1632_DATA_LINE;
		else
			PORT(HT1632_DATA) &= ~HT1632_DATA_LINE;

		PORT(HT1632_WR) &= ~HT1632_WR_LINE;
		asm("nop");
		asm("nop");
		PORT(HT1632_WR) |= HT1632_WR_LINE;
	}

	return;
}

void ht1632SendCmd(uint8_t cmd)
{
	PORT(HT1632_CS) &= ~HT1632_CS_LINE;
	ht1632SendBits(HT1632_IDBITS_CNT, HT1632_MODE_COMMAND);
	ht1632SendBits(HT1632_CMDBITS_CNT, cmd);
	ht1632SendBits(HT1632_CMDLASTBIT, 0);
	PORT(HT1632_CS) |= HT1632_CS_LINE;

	return;
}

void ht1632SendDataBuf(uint8_t *buf, uint8_t rotate)
{
	uint8_t i, j, k, data;

	PORT(HT1632_CS) &= ~HT1632_CS_LINE;
	ht1632SendBits(HT1632_IDBITS_CNT, HT1632_MODE_WRITE);
	ht1632SendBits(HT1632_ADDRBITS_CNT, 0);

	for (k = 0; k < MATRIX_NUMBER; k++) {
		for (i = 0; i < 8; i++) {
			data = 0;
			for (j = 0; j < 8; j++) {
				if (rotate) {
					if (buf[((MATRIX_NUMBER - 1 - k) << 3) + 7 - j] & (0x80 >> i))
						data |= (0x80 >> j);
				} else {
					if (buf[k * 8 + j] & (0x01 << i))
						data |= (0x80 >> j);
				}
			}
			ht1632SendBits(HT1632_DATABITS_CNT, data);
		}
	}

	PORT(HT1632_CS) |= HT1632_CS_LINE;

}

void ht1632Init(void)
{
	DDR(HT1632_DATA) |= HT1632_DATA_LINE;
	DDR(HT1632_CS) |= HT1632_CS_LINE;
	DDR(HT1632_WR) |= HT1632_WR_LINE;

	PORT(HT1632_DATA) |= HT1632_DATA_LINE;
	PORT(HT1632_CS) |= HT1632_CS_LINE;
	PORT(HT1632_WR) |= HT1632_WR_LINE;

	ht1632SendCmd(HT1632_CMD_SYS_ON);
	ht1632SendCmd(HT1632_CMD_LED_ON);
	ht1632SendCmd(HT1632_CMD_BLINK_OFF);
	ht1632SendCmd(HT1632_CMD_MASTER);
	ht1632SendCmd(HT1632_CMD_COM_NMOS_08);
	ht1632SendCmd(HT1632_CMD_DUTY);

	return;
}
