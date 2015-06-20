#ifndef DS1307_H
#define DS1307_H

#include <inttypes.h>

#define DS1307_ADDR		0b11010000

#define BD2D(x)			((x >> 4) * 10 + (x & 0x0F))
#define D2BD(x)			(((x / 10) << 4) + (x % 10))

enum {
	DS1307_SEC = 0,
	DS1307_MIN,
	DS1307_HOUR,
	DS1307_WDAY,
	DS1307_DATE,
	DS1307_MONTH,
	DS1307_YEAR,
	DS1307_ETM,
};

typedef struct {
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t wday;
	uint8_t date;
	uint8_t month;
	uint8_t year;
	uint8_t etm;
} RTC_type;

extern RTC_type rtc;

#define NOEDIT				0xFF

void readTime(void);
void stopEditTime(void);
void editTime(void);
void changeTime(int8_t diff);

#endif /* DS1307_H */
