#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

#define EEPROM_A_HOUR			((void*)0x00)
#define EEPROM_A_MIN			((void*)0x01)
#define EEPROM_A_DAYS			((void*)0x02)

#define EEPROM_SCREEN_ROTATE	((void*)0x04)
#define EEPROM_BIGNUM			((void*)0x05)
#define EEPROM_HOURZERO			((void*)0x06)

#define EEPROM_BR_MAX			((void*)0x08)

/* Text labels (maximum 15 byte followed by \0) */
#define EEPROM_LABELS			((uint8_t*)0x40)

/* Big numbers font (10 * 5 bytes) */
#define EEPROM_BIG_NUM_FONT		((uint8_t*)0x1C0)
/* Extra numbers font (10 * 6 bytes) */
#define EEPROM_EXTRA_NUM_FONT	((uint8_t*)0x180)

#define EEPROM_SIZE				0x200

enum {
	LABEL_SUNDAY,
	LABEL_MONDAY,
	LABEL_TUESDAY,
	LABEL_WEDNESDAY,
	LABEL_THURSDAY,
	LABEL_FRIDAY,
	LABEL_SADURDAY,

	LABEL_MO,
	LABEL_TU,
	LABEL_WE,
	LABEL_TH,
	LABEL_FR,
	LABEL_SA,
	LABEL_SU,

	LABEL_DECEMBER,
	LABEL_JANUARY,
	LABEL_FEBRUARY,
	LABEL_MARCH,
	LABEL_APRIL,
	LABEL_MAY,
	LABEL_JUNE,
	LABEL_JULY,
	LABEL_AUGUST,
	LABEL_SEPTEMBER,
	LABEL_OCTOBER,
	LABEL_NOVEMBER,

	LABEL_SECOND,
	LABEL_MINUTE,
	LABEL_HOUR,
	LABEL_WEEK,
	LABEL_DATA,
	LABEL_MONTH,
	LABEL_YEAR,

	LABEL_DEGREE,
	LABEL_TEMP1,
	LABEL_TEMP2,

	LABEL_Y,

	LABEL_END
};

#endif /* EEPROM_H */
