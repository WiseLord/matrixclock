#ifndef DS1307_H
#define DS1307_H

#include <inttypes.h>

#define DS1307_ADDR		0b11010000

#define BD2D(x)			((x >> 4) * 10 + (x & 0x0F))
#define D2BD(x)			(((x / 10) << 4) + (x % 10))

typedef enum {
	T_SEC   = 0,
	T_MIN   = 1,
	T_HOUR  = 2,
	T_WEEK  = 3,
	T_DAY   = 4,
	T_MONTH = 5,
	T_YEAR  = 6,
	T_NOEDIT = 7
} timeMode;

int8_t getTime(timeMode tm);
timeMode getEtm();

int8_t *readTime(void);
void stopEditTime(void);
uint8_t isETM(void);
void editTime(void);
void changeTime(int8_t diff);

#endif /* DS1307_H */
