#ifndef MTIMER_H
#define MTIMER_H

#include <avr/io.h>

/* Biper */
#define BEEPER_DDR			DDRD
#define BEEPER_PORT			PORTD
#define BEEPER_PIN			(1<<PD5)

#define TEMP_MEASURE_TIME	750
#define TEMP_POLL_INTERVAL	10000


void mTimerInit(void);

uint16_t getTempConvertTimer(void);
void setTempConvertTimer(uint16_t val);
uint16_t getTempStartTimer(void);
void setTempStartTimer(uint16_t val);

#endif /* MTIMER_H */
