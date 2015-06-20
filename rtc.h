#ifndef RTC_H
#define RTC_H

#include <inttypes.h>

#define RTC_I2C_ADDR		0xD0

#define BD2D(x)				((x >> 4) * 10 + (x & 0x0F))
#define D2BD(x)				(((x / 10) << 4) + (x % 10))

enum {
	RTC_SEC = 0,
	RTC_MIN,
	RTC_HOUR,
	RTC_WDAY,
	RTC_DATE,
	RTC_MONTH,
	RTC_YEAR,
	RTC_ETM,
};

typedef struct {
	int8_t sec;
	int8_t min;
	int8_t hour;
	int8_t wday;
	int8_t date;
	int8_t month;
	int8_t year;
	int8_t etm;
} RTC_type;

extern RTC_type rtc;

#define RTC_NOEDIT			0xFF

void rtcReadTime(void);
void rtcStopEditTime(void);
void rtcNextEditParam(void);
void rtcChangeTime(int8_t diff);

#endif /* RTC_H */
