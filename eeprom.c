#include "eeprom.h"

#include <avr/eeprom.h>

#include "matrix.h"

static EE_Param eep;

EE_Param *eeParamGet()
{
    return &eep;
}

void saveEeParam(void)
{
    eeprom_update_block(&eep, (void *)EEPROM_HOURSIGNAL, sizeof(EE_Param));
}

void changeCorrection(int8_t direction)
{
    eep.corr += direction;
    if (eep.corr > 55 || eep.corr < -55) {
        eep.corr = 0;
    }

    saveEeParam();
}

void displaySwitchHourSignal(void)
{
    eep.hourSignal = !eep.hourSignal;
    saveEeParam();
}

void displaySwitchHourZero(void)
{
    eep.hourZero = !eep.hourZero;
    saveEeParam();
}

void displaySwitchBigNum(void)
{
    if (++eep.bigNum >= NUM_SMALL) {
        eep.bigNum = NUM_NORMAL;
    }
    saveEeParam();
}

void displayChangeRotate(int8_t direction)
{
    eep.rotate += direction;
    saveEeParam();
}

void changeBrightness(int8_t direction)
{
    eep.brMax += direction;
    eep.brMax &= 0x0F;

    saveEeParam();
}
