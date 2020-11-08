#include "matrix.h"

#include "fonts.h"
#include "eeprom.h"
#include "mtimer.h"

#include "pins.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

static int16_t _col;                        // Current position

uint8_t fb[MATRIX_BUFFER_SIZE];

static uint8_t fbRaw[MATRIX_BUFFER_SIZE];
static uint8_t fbNew[MATRIX_BUFFER_SIZE];

static char fbStr[MATRIX_STRING_LEN];
static uint8_t fbStrPos;

static char *ptrStr = fbStr;
static volatile uint8_t chCol = 0;
static volatile uint8_t scrollData = 0x00;

static volatile uint8_t scrollMode = MATRIX_SCROLL_OFF;

static void matrixLoadCharFb(uint8_t code, uint8_t numSize)
{
    uint8_t i;
    uint8_t data;
    const uint8_t *oft;
    uint8_t chOft;

    const uint8_t *font = font_cp1251_08;
    uint8_t width = MATRIX_FONT_WIDTH;
    uint8_t memType = MATRIX_FONT_PROGMEM;

    chOft = code - '0';

    if (numSize == NUM_SMALL) {
        font = font_smallnum;
        width = MATRIX_SMALLNUM_WIDTH;
#if MATRIX_CNT == 4
    } else if (numSize == NUM_EXTRA) {
        font = (uint8_t *)EEPROM_EXTRA_NUM_FONT;
        width = MATRIX_EXTRANUM_WIDTH;
        memType = MATRIX_FONT_EEPROM;
#endif
    } else if (numSize == NUM_BIG) {
        font = (uint8_t *)EEPROM_BIG_NUM_FONT;
        width = MATRIX_BIGNUM_WIDTH;
        memType = MATRIX_FONT_EEPROM;
    } else {
        chOft = code - ' ';
        // TODO: Remove it with full font
        if (code > 128)
            chOft -= 0x20;
    }

    for (i = 0; i < width; i++) {
        if (numSize != NUM_NORMAL && (code < '0' || code > '9')) {
            data = 0x00;
        } else {
            oft = font + chOft * width + i;
            if (memType == MATRIX_FONT_EEPROM)
                data = eeprom_read_byte(oft);
            else if (memType == MATRIX_FONT_PROGMEM)
                data = pgm_read_byte(oft);
            else
                data = *oft;
        }
        if (data != VOID)
            fbNew[_col++] = data;
    }
    if (_col < MATRIX_BUFFER_SIZE - 1)
        fbNew[_col++] = 0x00;
}

static void matrixLoadScrollChar(uint8_t ch)
{
    fbStr[fbStrPos++] = ch;
    fbStr[fbStrPos] = '\0';
}

void matrixInit(void)
{
    matrixInitDriver();
    scrollTimer = eeParamGet()->scrollInterval;
}

void matrixSetBrightness(uint8_t brightness)
{
    if (scrollMode == MATRIX_SCROLL_OFF) {
#if defined(_HT1632)
        ht1632SendCmd(HT1632_CMD_DUTY | brightness);
#else
        max7219SendCmd(MAX7219_INTENSITY, brightness);
#endif
    }
}

void matrixSwitchBuf(uint32_t mask, int8_t effect)
{
    uint8_t i, j;
    uint8_t rsBit;
    uint8_t lsBit;

    rsBit = 0x80;
    lsBit = 0x01;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < MATRIX_BUFFER_SIZE; j++) {
            if (mask & (1UL << j)) {
                switch (effect) {
                case MATRIX_EFFECT_SCROLL_DOWN:
                    fb[j] <<= 1;
                    if (fbNew[j] & rsBit)
                        fb[j] |= 0x01;
                    break;
                case MATRIX_EFFECT_SCROLL_UP:
                    fb[j] >>= 1;
                    if (fbNew[j] & lsBit)
                        fb[j] |= 0x80;
                    break;
                case MATRIX_EFFECT_SCROLL_BOTH:
                    if (j & 0x01) {
                        fb[j] <<= 1;
                        if (fbNew[j] & rsBit)
                            fb[j] |= 0x01;
                    } else {
                        fb[j] >>= 1;
                        if (fbNew[j] & lsBit)
                            fb[j] |= 0x80;
                    }
                    break;
                default:
                    fb[j] = fbNew[j];
                    break;
                }
            }
        }
        _delay_ms(25);
        matrixWrite();
        rsBit >>= 1;
        lsBit <<= 1;
    }

    fbStrPos = 0;
}

void matrixSetX(int16_t x)
{
    _col = x;
}

void matrixScrollAndADCInit(void)
{
    // Enable Timer2 overflow interrupt and set prescaler to 1024 (7812 Hz)
#if defined(_atmega8)
    TIMSK |= (1 << TOIE2);
    TCCR2 = (1 << CS22) | (1 << CS21) | (1 << CS20);
#else
    TIMSK2 = (1 << TOIE2);
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
#endif

    // Adjust result to left, use VCC as Vref and set ADC channel
    ADMUX = (1 << ADLAR) | (0 << REFS1) | (1 << REFS0) | ADC_CHANNEL;
    // Enable ADC
    ADCSRA |= (1 << ADEN);
}

// Interrupt will be executed 7812 / 256 = 30 times/sec
ISR (TIMER2_OVF_vect)
{
    int8_t i;
    uint8_t code;

    if (scrollMode == MATRIX_SCROLL_ON) {
        if (*ptrStr) {
            if (chCol < 5) {
                code = *ptrStr;
                code -= (code > 128 ? 0x40 : 0x20);
                scrollData = pgm_read_byte(font_cp1251_08 + code * 5 + chCol);
                if (scrollData == VOID)
                    chCol = 5;
            }
            if (++chCol >= 6) {
                chCol = 0;
                ptrStr++;
                scrollData = 0x00;
            }

            for (i = 0; i < MATRIX_BUFFER_SIZE - 1; i++)
                fb[i] = fb[i + 1];
            fb[MATRIX_BUFFER_SIZE - 1] = scrollData;
            matrixWrite();
        } else {
            scrollMode = MATRIX_SCROLL_OFF;
            scrollTimer = eeParamGet()->scrollInterval;
        }
    } else {
        ptrStr = fbStr;
        chCol = 0;
        scrollData = 0x00;
    }

    // Start ADC conversion to get brightness from photoresistor
    ADCSRA |= 1 << ADSC;
}

void matrixHwScroll(uint8_t status)
{
    uint8_t i;

    for (i = 0; i < sizeof(fbNew); i++)
        fbNew[i] = 0x00;

    for (i = 0; i < 6; i++)
        matrixLoadScrollChar(' ');

    scrollMode = status;
    scrollTimer = eeParamGet()->scrollInterval;
}

uint8_t matrixGetScrollMode(void)
{
    return scrollMode;
}

void matrixScrollAddString(char *string)
{
    while (*string)
        matrixLoadScrollChar(*string++);
}

void matrixScrollAddStringEeprom(uint8_t *string)
{
    uint8_t ch;
    uint8_t i = 0;

    ch = eeprom_read_byte(&string[i++]);
    while (ch) {
        matrixLoadScrollChar(ch);
        ch = eeprom_read_byte(&string[i++]);
    }
}

void matrixFbNewAddString(char *string, uint8_t numSize)
{
    while (*string)
        matrixLoadCharFb(*string++, numSize);
}

void matrixFbNewAddStringEeprom(uint8_t *string)
{
    fbStrPos = 0;
    matrixScrollAddStringEeprom(string);
    matrixFbNewAddString(fbStr, NUM_NORMAL);
}

inline uint8_t swapBits(uint8_t data) __attribute__((always_inline));
inline uint8_t swapBits(uint8_t data)
{
    data = ((data >> 4) & 0x0F) | ((data << 4) & 0xF0);
    data = ((data >> 2) & 0x33) | ((data << 2) & 0xCC);
    data = ((data >> 1) & 0x55) | ((data << 1) & 0xAA);

    return data;
}

void matrixWrite(void)
{
    uint8_t m, mp, mn;
    uint8_t r, rp, rn;

    uint8_t data;
    uint8_t *pRaw = fbRaw;

    uint8_t bit;

    // Rotate magic
    uint8_t rotate = eeParamGet()->rotate;

    for (mp = 0, mn = MATRIX_CNT - 1; mp < MATRIX_CNT; mp++, mn--) {
        m = (rotate & BIT_MIRROR) ? mn : mp;

        for (rp = 0, rn = 7; rp < 8; rp++, rn--) {
            r = (rotate & BIT_SCAN) ? rn : rp;

            if (rotate & BIT_ROTATE) {
                data = 0;
                uint8_t bs = 0x01;
                for (bit = 0; bit < 8; bit++) {
                    if (fb[m * 8 + bit] & (1 << r))
                        data |= bs;
                    bs <<= 1;
                }
            } else {
                data = fb[m * 8 + r];
            }

            if (rotate & BIT_SWAP)
                data = swapBits(data);

            *pRaw++ = data;
        }
    }

    matrixUpdate(fbRaw);
}
