#include "ht1632.h"

#include <avr/pgmspace.h>

static const uint8_t initCmdSeq[] PROGMEM = {
	HT1632_CMD_SYS_ON,
	HT1632_CMD_LED_ON,
	HT1632_CMD_BLINK_OFF,
	HT1632_CMD_MASTER,
	HT1632_CMD_COM_NMOS_08,
	HT1632_CMD_DUTY,
};

static void ht1632SendBits(uint8_t bits, uint8_t cnt)
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
	ht1632SendBits(HT1632_MODE_COMMAND, 3);
	ht1632SendBits(cmd, 8);
	ht1632SendBits(0, 1);
	PORT(HT1632_CS) |= HT1632_CS_LINE;

	return;
}

void ht1632SendDataBuf(uint8_t *buf, uint8_t rotate)
{
	uint8_t i, j, k, data;

	PORT(HT1632_CS) &= ~HT1632_CS_LINE;
	ht1632SendBits(HT1632_MODE_WRITE, 3);
	ht1632SendBits(0, 7);

	for (k = 0; k < 4; k++) {
		for (i = 0; i < 8; i++) {
			data = 0;
			for (j = 0; j < 8; j++) {
				if (rotate) {
					if (buf[((3 - k) << 3) + 7 - j] & (0x80 >> i))
						data |= (0x80 >> j);
				} else {
					if (buf[k * 8 + j] & (0x01 << i))
						data |= (0x80 >> j);
				}
			}
			ht1632SendBits(data, 8);
		}
	}

	PORT(HT1632_CS) |= HT1632_CS_LINE;

}

void ht1632Init(void)
{
	uint8_t i;

	DDR(HT1632_DATA) |= HT1632_DATA_LINE;
	DDR(HT1632_CS) |= HT1632_CS_LINE;
	DDR(HT1632_WR) |= HT1632_WR_LINE;

	PORT(HT1632_DATA) |= HT1632_DATA_LINE;
	PORT(HT1632_CS) |= HT1632_CS_LINE;
	PORT(HT1632_WR) |= HT1632_WR_LINE;

	for (i = 0; i < sizeof(initCmdSeq); i++)
		ht1632SendCmd(pgm_read_byte(&initCmdSeq[i]));

	return;
}
