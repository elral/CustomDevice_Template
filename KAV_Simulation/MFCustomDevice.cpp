#include "MFCustomDevice.h"
#include "commandmessenger.h"
#include "allocateMem.h"
#include "MFEEPROM.h"
extern MFEEPROM MFeeprom;

/* **********************************************************************************
    The custom device pins, type and configuration is stored in the EEPROM
    While loading the config the adresses in the EEPROM are transferred to the constructor
    Within the constructor you have to copy the EEPROM content to a buffer
    and evaluate him. The buffer is used for all 3 types (pins, type configuration),
    so do it step by step.
    The max size of the buffer is defined here. It must be the size of the
    expected max length of these strings.
    E.g. 6 pins are required, each pin could have two characters (two digits),
    each pins are delimited by "|" and the string is NULL terminated.
    -> (6 * 2) + 5 + 1 = 18 bytes is the maximum.
    The custom type is "MyCustomDevice", which means 14 characters plus NULL = 15
    The configuration is "myConfig", which means 8 characters plus NULL = 9
    The maximum characters to be expected is 18, so MEMLEN_STRING_BUFFER has to be at least 18

    For the KAV displays:
    3 Pins = 9
    "KAV_EFIS" / "KAV_FCU" / "KAV_BATTERY" / "KAV_RAD_TCAS" / "KAV_RUDDER" = 11
    no config = 0
********************************************************************************** */

#define MEMLEN_STRING_BUFFER 15

// reads a string from EEPROM at given address which is '.' terminated and saves it to the buffer
bool MFCustomDevice::getStringFromEEPROM(uint16_t addreeprom, char *buffer)
{
    char    temp    = 0;
    uint8_t counter = 0;
    do {
        temp              = MFeeprom.read_byte((addreeprom)++); // read the first character
        buffer[counter++] = temp;                               // save character and locate next buffer position
        if (counter >= MEMLEN_STRING_BUFFER) {                  // nameBuffer will be exceeded
            return false;                                       // abort copying to buffer
        }
    } while (temp != '.');      // reads until limiter '.' and locates the next free buffer position
    buffer[counter - 1] = 0x00; // replace '.' by NULL, terminates the string
    return true;
}

MFCustomDevice::MFCustomDevice()
{
    _initialized = false;
}

/* **********************************************************************************
    Within the connector pins, a device name and a config string can be defined
    These informations are stored in the EEPROM like for the other devices.
    While reading the config from the EEPROM this function is called.
    It is the first function which will be called for the custom device.
    If it fits into the memory buffer, the constructor for the customer device
    will be called
********************************************************************************** */

void MFCustomDevice::attach(uint16_t adrPin, uint16_t adrType, uint16_t adrConfig)
{
    if (adrPin == 0) return;

    /* **********************************************************************************
        Do something which is required to setup your custom device
    ********************************************************************************** */

    char   *params, *p = NULL;
    char    parameter[MEMLEN_STRING_BUFFER];
    uint8_t _pin1, _pin2, _pin3;

    /* **********************************************************************************
        Read the Type from the EEPROM, copy it into a buffer and evaluate it
        The string get's NOT stored as this would need a lot of RAM, instead a variable
        is used to store the type
    ********************************************************************************** */
    getStringFromEEPROM(adrType, parameter);
    if (strcmp(parameter, "KAV_FCU") == 0)
        _lcdType = KAV_FCU;
    if (strcmp(parameter, "KAV_EFIS") == 0)
        _lcdType = KAV_EFIS;
    if (strcmp(parameter, "KAV_BATTERY") == 0)
        _lcdType = KAV_BATTERY;
    if (strcmp(parameter, "KAV_RAD_TCAS") == 0)
        _lcdType = KAV_RAD_TCAS;
    if (strcmp(parameter, "KAV_RUDDER") == 0)
        _lcdType = KAV_RUDDER;

    if (_lcdType == KAV_FCU) {
        /* **********************************************************************************
            Check if the device fits into the device buffer
        ********************************************************************************** */
        if (!FitInMemory(sizeof(KAV_A3XX_FCU_LCD))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("FCU LCD does not fit in Memory"));
            return;
        }

        /* **********************************************************************************************
            Read the pins from the EEPROM, copy them into a buffer
            If you have set '"isI2C": true' in the device.json file, the first value is the I2C address
        ********************************************************************************************** */
        getStringFromEEPROM(adrPin, parameter);
        /* **********************************************************************************************
            split the pins up into single pins. As the number of pins could be different between
            multiple devices, it is done here.
        ********************************************************************************************** */
        params = strtok_r(parameter, "|", &p);
        _pin1  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin2  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin3  = atoi(params);

        /* **********************************************************************************
            Next call the constructor of your custom device
            adapt it to the needs of your constructor
        ********************************************************************************** */
        _FCU_LCD = new (allocateMemory(sizeof(KAV_A3XX_FCU_LCD))) KAV_A3XX_FCU_LCD(_pin2, _pin3, _pin1);
        _FCU_LCD->attach(_pin2, _pin3, _pin1);
        _initialized = true;
    } else if (_lcdType == KAV_EFIS) {
        /* **********************************************************************************
            Check if the device fits into the device buffer
        ********************************************************************************** */
        if (!FitInMemory(sizeof(KAV_A3XX_FCU_LCD))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("EFIS LCD does not fit in Memory"));
            return;
        }

        /* **********************************************************************************************
            Read the pins from the EEPROM, copy them into a buffer
            If you have set '"isI2C": true' in the device.json file, the first value is the I2C address
        ********************************************************************************************** */
        getStringFromEEPROM(adrPin, parameter);
        /* **********************************************************************************************
            split the pins up into single pins. As the number of pins could be different between
            multiple devices, it is done here.
        ********************************************************************************************** */
        params = strtok_r(parameter, "|", &p);
        _pin1  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin2  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin3  = atoi(params);

        /* **********************************************************************************
            Next call the constructor of your custom device
            adapt it to the needs of your constructor
        ********************************************************************************** */
        _EFIS_LCD = new (allocateMemory(sizeof(KAV_A3XX_EFIS_LCD))) KAV_A3XX_EFIS_LCD(_pin2, _pin3, _pin1);
        _EFIS_LCD->attach(_pin2, _pin3, _pin1);
        _initialized = true;
    } else if (_lcdType == KAV_BATTERY) {
        /* **********************************************************************************
            Check if the device fits into the device buffer
        ********************************************************************************** */
        if (!FitInMemory(sizeof(KAV_A3XX_BATTERY_LCD))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("BATTERY LCD does not fit in Memory"));
            return;
        }

        /* **********************************************************************************************
            Read the pins from the EEPROM, copy them into a buffer
            If you have set '"isI2C": true' in the device.json file, the first value is the I2C address
        ********************************************************************************************** */
        getStringFromEEPROM(adrPin, parameter);
        /* **********************************************************************************************
            split the pins up into single pins. As the number of pins could be different between
            multiple devices, it is done here.
        ********************************************************************************************** */
        params = strtok_r(parameter, "|", &p);
        _pin1  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin2  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin3  = atoi(params);

        /* **********************************************************************************
            Next call the constructor of your custom device
            adapt it to the needs of your constructor
        ********************************************************************************** */
        _BATTERY_LCD = new (allocateMemory(sizeof(KAV_A3XX_BATTERY_LCD))) KAV_A3XX_BATTERY_LCD(_pin2, _pin3, _pin1);
        _BATTERY_LCD->attach(_pin2, _pin3, _pin1);
        _initialized = true;
    } else if (_lcdType == KAV_RAD_TCAS) {
        /* **********************************************************************************
            Check if the device fits into the device buffer
        ********************************************************************************** */
        if (!FitInMemory(sizeof(KAV_A3XX_RAD_TCAS_LCD))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("RAD/TCAS LCD does not fit in Memory"));
            return;
        }

        /* **********************************************************************************************
            Read the pins from the EEPROM, copy them into a buffer
            If you have set '"isI2C": true' in the device.json file, the first value is the I2C address
        ********************************************************************************************** */
        getStringFromEEPROM(adrPin, parameter);
        /* **********************************************************************************************
            split the pins up into single pins. As the number of pins could be different between
            multiple devices, it is done here.
        ********************************************************************************************** */
        params = strtok_r(parameter, "|", &p);
        _pin1  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin2  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin3  = atoi(params);

        /* **********************************************************************************
            Next call the constructor of your custom device
            adapt it to the needs of your constructor
        ********************************************************************************** */
        _RAD_TCAS_LCD = new (allocateMemory(sizeof(KAV_A3XX_RAD_TCAS_LCD))) KAV_A3XX_RAD_TCAS_LCD(_pin2, _pin3, _pin1);
        _RAD_TCAS_LCD->attach(_pin2, _pin3, _pin1);
        _initialized = true;
    } else if (_lcdType == KAV_RUDDER) {
        /* **********************************************************************************
            Check if the device fits into the device buffer
        ********************************************************************************** */
        if (!FitInMemory(sizeof(KAV_A3XX_RUDDER_LCD))) {
            // Error Message to Connector
            cmdMessenger.sendCmd(kStatus, F("Rudder LCD does not fit in Memory"));
            return;
        }

        /* **********************************************************************************************
            Read the pins from the EEPROM, copy them into a buffer
            If you have set '"isI2C": true' in the device.json file, the first value is the I2C address
        ********************************************************************************************** */
        getStringFromEEPROM(adrPin, parameter);
        /* **********************************************************************************************
            split the pins up into single pins. As the number of pins could be different between
            multiple devices, it is done here.
        ********************************************************************************************** */
        params = strtok_r(parameter, "|", &p);
        _pin1  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin2  = atoi(params);
        params = strtok_r(NULL, "|", &p);
        _pin3  = atoi(params);

        /* **********************************************************************************
            Next call the constructor of your custom device
            adapt it to the needs of your constructor
        ********************************************************************************** */
        _RUDDER_LCD = new (allocateMemory(sizeof(KAV_A3XX_RUDDER_LCD))) KAV_A3XX_RUDDER_LCD(_pin2, _pin3, _pin1);
        _RUDDER_LCD->attach(_pin2, _pin3, _pin1);
        _initialized = true;
    } else {
        cmdMessenger.sendCmd(kStatus, F("Custom Device is not supported by this firmware version"));
    }
}

void MFCustomDevice::detach()
{
    _initialized = false;
    if (_lcdType == KAV_FCU) {
        _FCU_LCD->detach();
    } else if (_lcdType == KAV_EFIS) {
        _EFIS_LCD->detach();
    }
}

/* **********************************************************************************
    Within in loop() the update() function is called regularly
    Within the loop() you can define a time delay where this function gets called
    or as fast as possible. See comments in loop().
    It is only needed if you have to update your custom device without getting
    new values from the connector.
    Otherwise just make a return; in the calling function.
    It gets called from CustomerDevice::update()
********************************************************************************** */
void MFCustomDevice::update()
{
    if (!_initialized) return;
    /* **********************************************************************************
        Do something if required
        -> Nothing todo for this device
    ********************************************************************************** */
}

/* **********************************************************************************
    If an output for the custom device is defined in the connector,
    this function gets called when a new value is available.
    It gets called from CustomerDevice::OnSet()
********************************************************************************** */
void MFCustomDevice::set(int16_t messageID, char *setPoint)
{
    if (!_initialized) return;

    if (_lcdType == KAV_FCU)
        _FCU_LCD->set(messageID, setPoint);
    else if (_lcdType == KAV_EFIS)
        _EFIS_LCD->set(messageID, setPoint);
}
