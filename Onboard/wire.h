/*
    Modfidficatopn of TwoWire.h - TWI/I2C library for Arduino & Wiring
    Copyright (c) 2006 Nicholas Zambetti
    for Redpitaya by Robert Reichert.
    This library is free software.
*/

#ifndef TwoWire_h
#define TwoWire_h

#include <inttypes.h>

#ifndef I2C_BUFFER_LENGTH
#define I2C_BUFFER_LENGTH 128
#define I2C_SLAVE_FORCE 0x0706
#endif

class TwoWire
{
private:
    static uint8_t rxBuffer[];
    static uint8_t  rxBufferIndex;
    static uint8_t  rxBufferLength;

    static uint8_t txAddress;
    static uint8_t txBuffer[];
    static uint8_t  txBufferIndex;
    static uint8_t  txBufferLength;

    static void (*user_onRequest)(void);
    static void (*user_onReceive)(uint8_t);
    static void onRequestService(void);
    static void onReceiveService(uint8_t*, uint8_t);

    static int device_file;

public:
    TwoWire();
    void    begin();
    //void    setClock(uint32_t);
    void    beginTransmission(uint8_t);
    void    beginTransmission(int);
    uint8_t endTransmission(void);

    uint8_t requestFrom(uint8_t, uint8_t);
    uint8_t requestFrom(int, int);

    uint8_t writeData(uint8_t);
    uint8_t writeData(const uint8_t*, uint8_t);
    int    available(void);
    int    readData(void);
    void   flush(void);

};

#endif