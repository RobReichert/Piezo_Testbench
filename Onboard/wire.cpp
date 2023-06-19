/*
    Modfidficatopn of TwoWire.h - TWI/I2C library for Arduino & Wiring
    Copyright (c) 2006 Nicholas Zambetti
    for Redpitaya by Robert Reichert.
    This library is free software.
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <inttypes.h>

#include "wire.h"

// Initialize Class Variables //////////////////////////////////////////////////

uint8_t TwoWire::rxBuffer[I2C_BUFFER_LENGTH];
uint8_t  TwoWire::rxBufferIndex  = 0;
uint8_t  TwoWire::rxBufferLength = 0;

uint8_t TwoWire::txAddress = 0;
uint8_t TwoWire::txBuffer[I2C_BUFFER_LENGTH];
uint8_t  TwoWire::txBufferIndex  = 0;
uint8_t  TwoWire::txBufferLength = 0;

void (*TwoWire::user_onRequest)(void);
void (*TwoWire::user_onReceive)(uint8_t);

int TwoWire::device_file=0;

// Constructors ////////////////////////////////////////////////////////////////

TwoWire::TwoWire() { }

// Public Methods //////////////////////////////////////////////////////////////

void TwoWire::begin(void)
{
    /* Open the device */
	device_file = open("/dev/i2c-0", O_RDWR);
    flush();
}

/*void TwoWire::setClock(uint32_t frequency)
{
    //not possible in this version
    use bootargs setup according https://forum.redpitaya.com/viewtopic.php?t=1746
}*/

uint8_t TwoWire::requestFrom(uint8_t address, uint8_t quantity)
{
    if (quantity > I2C_BUFFER_LENGTH)
    {
        quantity = I2C_BUFFER_LENGTH;
    }
    ioctl(device_file, I2C_SLAVE_FORCE, address);
    uint8_t read_sice = read(device_file, rxBuffer, quantity);
    rxBufferIndex  = 0;
    rxBufferLength = read_sice;
    return read_sice;
}

uint8_t TwoWire::requestFrom(int address, int quantity)
{
    return requestFrom((uint8_t)address, (uint8_t)quantity);
}

void TwoWire::beginTransmission(uint8_t address)
{
    txAddress      = address;
    txBufferIndex  = 0;
    txBufferLength = 0;
}

void TwoWire::beginTransmission(int address)
{
    beginTransmission((uint8_t)address);
}

uint8_t TwoWire::endTransmission(void)
{
    int8_t ret = ioctl(device_file, I2C_SLAVE_FORCE, txAddress);
    write(device_file, txBuffer, txBufferLength);
    txBufferIndex  = 0;
    txBufferLength = 0;
    return ret;
}

uint8_t TwoWire::writeData(uint8_t data)
{
    if (txBufferLength >= I2C_BUFFER_LENGTH)
    {
        return 0;
    }
    txBuffer[txBufferIndex] = data;
    ++txBufferIndex;
    txBufferLength = txBufferIndex;
    return 1;
}

uint8_t TwoWire::writeData(const uint8_t* data, uint8_t quantity)
{
    for (uint8_t i = 0; i < quantity; ++i)
    {
        if (!writeData(data[i]))
        {
            return i;
        }
    }
    return quantity;
}

int TwoWire::available(void)
{
    int result = rxBufferLength - rxBufferIndex;
    return result;
}

int TwoWire::readData(void)
{
    int value = -1;
    if (rxBufferIndex < rxBufferLength)
    {
        value = rxBuffer[rxBufferIndex];
        ++rxBufferIndex;
    }
    return value;
}

void TwoWire::flush(void)
{
    rxBufferIndex  = 0;
    rxBufferLength = 0;
    txBufferIndex  = 0;
    txBufferLength = 0;
}

// Preinstantiate Objects //////////////////////////////////////////////////////

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_TWOWIRE)
TwoWire Wire;
#endif