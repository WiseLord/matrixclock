#include "mtimer.h"

#include <avr/interrupt.h>
#include "pins.h"

// Temperature/pressure/humidity sensor poll timer
uint8_t sensTimer = 0;
// Scroll interval timer
uint8_t scrollTimer = 0;
// Alarm timer
uint16_t alarmTimer = 0;

// Beeper timer
static volatile uint8_t beepTimer = 0;

// Seconds timer
static volatile uint8_t secTimer = TIME_SEC;

// Command buffer
static volatile uint8_t cmdBuf;

void mTimerInit(void)
{
#if defined(_atmega8)
    TIMSK |= (1 << TOIE0);                              // Enable Timer0 overflow interrupt
    TCCR0 = (1 << CS02) | (0 << CS01) | (0 << CS00);    // Set timer prescaller to 256 (31250Hz)
#else
    TIMSK0 |= (1 << TOIE0);                             // Enable Timer0 overflow interrupt
    TCCR0B = (1 << CS02) | (0 << CS01) | (0 << CS00);   // Set timer prescaller to 256 (31250Hz)
#endif

    OUT(BEEPER);
    SET(BEEPER);

    // Setup buttons as inputs with pull-up resistors
    IN(BUTTON_1);
    IN(BUTTON_2);
    IN(BUTTON_3);
    SET(BUTTON_1);
    SET(BUTTON_2);
    SET(BUTTON_3);

    cmdBuf = BTN_STATE_0;
}

ISR (TIMER0_OVF_vect)                                   // 31250 / (256 - 131) = 250 polls/sec
{
    TCNT0 = 131;

    static int16_t btnCnt = 0;                          // Buttons press duration value
    static uint8_t btnPrev = 0;                         // Previous buttons state

    uint8_t btnNow = BTN_STATE_0;

    if (!READ(BUTTON_1))
        btnNow |= BTN_0;
    if (!READ(BUTTON_2))
        btnNow |= BTN_1;
    if (!READ(BUTTON_3))
        btnNow |= BTN_2;

    // If button event has happened, place it to command buffer
    if (btnNow) {
        if (btnNow == btnPrev) {
            btnCnt++;
            if (btnCnt == LONG_PRESS)
                cmdBuf = (btnPrev << 4);
        } else {
            btnPrev = btnNow;
        }
    } else {
        if ((btnCnt > SHORT_PRESS) && (btnCnt < LONG_PRESS))
            cmdBuf = btnPrev;
        btnCnt = 0;
    }

    // 1 second intervals
    if (secTimer) {
        secTimer--;
    } else {
        secTimer = TIME_SEC;
        // Temperature
        if (sensTimer)
            sensTimer--;
        if (scrollTimer > 1)
            scrollTimer--;

        if (alarmTimer)
            alarmTimer--;
    }

    // Beeper
    if (beepTimer) {
        beepTimer--;
    } else {
        if (alarmTimer)
            beepTimer = TIME_SEC;
    }

    if ((beepTimer & 0x0E) > 8) {
        if (secTimer > TIME_SEC / 2)
            CLR(BEEPER);
    } else {
        SET(BEEPER);
    }
}

void startBeeper(uint8_t time)
{
    beepTimer = time;
    secTimer = TIME_SEC;
    alarmTimer = 0;
}

uint8_t getBtnCmd(void)
{
    uint8_t ret = cmdBuf;
    cmdBuf = BTN_STATE_0;

    return ret;
}
