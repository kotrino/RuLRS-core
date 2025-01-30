#include "rulrs_eeprom.h"
#include "targets.h"
#include "logging.h"

#if !defined(TARGET_NATIVE)
#include <EEPROM.h>

void
RULRS_EEPROM::Begin()
{
    EEPROM.begin(RESERVED_EEPROM_SIZE);
}

uint8_t
RULRS_EEPROM::ReadByte(const uint32_t address)
{
    if (address >= RESERVED_EEPROM_SIZE)
    {
        // адрес вне допустимого диапазона
        ERRLN("EEPROM address is out of bounds");
        return 0;
    }
    return EEPROM.read(address);
}

void
RULRS_EEPROM::WriteByte(const uint32_t address, const uint8_t value)
{
    if (address >= RESERVED_EEPROM_SIZE)
    {
        // адрес вне допустимого диапазона
        ERRLN("EEPROM address is out of bounds");
        return;
    }
    EEPROM.write(address, value);
}

void
RULRS_EEPROM::Commit()
{
    if (!EEPROM.commit())
    {
      ERRLN("EEPROM commit failed");
    }
}

#endif /* !TARGET_NATIVE */