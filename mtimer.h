#ifndef MTIMER_H
#define MTIMER_H

#include <avr/io.h>

void mTimerInit(void);

uint16_t getTempTimer();
void setTempTimer(uint16_t val);

#endif /* MTIMER_H */
