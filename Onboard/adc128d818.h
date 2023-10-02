#ifndef ADC128D818_H
#define ADC128D818_H

#include "wire.h"
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

#define DEBUG 0
#define MAX_ITER_NOT_READY 10
#define MAX_MS_WAIT_FOR_NOT_BUSY  200 // ~ 2 x 8 (channels) x 12.2 ms ( convertion time for each channel - see 9.2.2.2.6)

// busy status
#define ADC128D818_STATUS_BUSY_BIT       0x01  // e.g. STAUS = "Reading"
#define ADC128D818_STATUS_NOT_READY_BIT  0x02  // e.g. while power up chip

// device address
#define ADC128D818_ADDRESS_LOW_LOW 0x1D // which one to choose depends on the value of A0 and A1
#define ADC128D818_ADDRESS_LOW_MID 0x1E
#define ADC128D818_ADDRESS_LOW_HIGH 0x1F
#define ADC128D818_ADDRESS_MID_LOW 0x2D
#define ADC128D818_ADDRESS_MID_MID 0x2E
#define ADC128D818_ADDRESS_MID_HIGH 0x2F
#define ADC128D818_ADDRESS_HIGH_LOW 0x35
#define ADC128D818_ADDRESS_HIGH_MID 0x36
#define ADC128D818_ADDRESS_HIGH_HIGH 0x37

// operation mode
#define ADC128D818_OPERATION_MODE_0 0x00 // Single ended with temp.: IN0..IN6 plus temp 
#define ADC128D818_OPERATION_MODE_1 0x01 // Single ended: IN0..IN7 //should use this
#define ADC128D818_OPERATION_MODE_2 0x02 // Differential: IN0-IN1, IN3-IN2, IN4-IN5, IN7-IN6
#define ADC128D818_OPERATION_MODE_3 0x03  // Mixed: IN0..IN3, IN4-IN5, IN7-IN6

// Vref internal or external
#define ADC128D818_VREF_INT 0x00
#define ADC128D818_VREF_EXT 0x01// should choose external ref, Vref = 5V

// channel limit
#define ADC128D818_LIMIT_HIGH   0x00
#define ADC128D818_LIMIT_LOW    0x01

#define ADC128D818_CHANNEL_IN0   0x00
#define ADC128D818_CHANNEL_IN1   0x01
#define ADC128D818_CHANNEL_IN2   0x02
#define ADC128D818_CHANNEL_IN3   0x03
#define ADC128D818_CHANNEL_IN4   0x04
#define ADC128D818_CHANNEL_IN5   0x05
#define ADC128D818_CHANNEL_IN6   0x06
#define ADC128D818_CHANNEL_IN7   0x07
#define ADC128D818_CHANNEL_TEMP  0x07

#define ADC128D818_INT_IN0  (char)~(0x01 <<0)
#define ADC128D818_INT_IN1  (char)~(0x01 <<1)
#define ADC128D818_INT_IN2  (char)~(0x01 <<2)
#define ADC128D818_INT_IN3  (char)~(0x01 <<3)
#define ADC128D818_INT_IN4  (char)~(0x01 <<4)
#define ADC128D818_INT_IN5  (char)~(0x01 <<5)
#define ADC128D818_INT_IN6  (char)~(0x01 <<6)
#define ADC128D818_INT_IN7  (char)~(0x01 <<7)
#define ADC128D818_INT_TEMP  (char)~(0x01 <<7)
#define ADC128D818_INT_ALL  0x00

#define ADC128D818_ENABLE_IN0  (char)~(0x01 <<0)
#define ADC128D818_ENABLE_IN1  (char)~(0x01 <<1)
#define ADC128D818_ENABLE_IN2  (char)~(0x01 <<2)
#define ADC128D818_ENABLE_IN3  (char)~(0x01 <<3)
#define ADC128D818_ENABLE_IN4  (char)~(0x01 <<4)
#define ADC128D818_ENABLE_IN5  (char)~(0x01 <<5)
#define ADC128D818_ENABLE_IN6  (char)~(0x01 <<6)
#define ADC128D818_ENABLE_IN7  (char)~(0x01 <<7)
#define ADC128D818_ENABLE_TEMP  ~(0x01 <<7)
#define ADC128D818_ENABLE_ALL  0x00

// register address
#define ADC128D818_REG_Configuration_Register            0x00
#define ADC128D818_REG_Interrupt_Status_Register         0x01
#define ADC128D818_REG_Interrupt_Mask_Register           0x03
#define ADC128D818_REG_Conversion_Rate_Register          0x07
#define ADC128D818_REG_Channel_Disable_Register          0x08
#define ADC128D818_REG_One_Shot_Register                 0x09
#define ADC128D818_REG_Deep_Shutdown_Register            0x0A
#define ADC128D818_REG_Advanced_Configuration_Register   0x0B
#define ADC128D818_REG_Busy_Status_Register              0x0C
#define ADC128D818_REG_Channel_Readings_Registers        0x20 // in mode 1: 0x20  IN0 0x27  IN7
#define ADC128D818_REG_Temperature_Register              0x27 // in mode 0 CH7 is not external is the internal temperature sensor
#define ADC128D818_REG_Limit_Registers                   0x2A
#define ADC128D818_REG_Manufacturer_ID_Register          0x3E
#define REG_Revision_ID_Register              0x3F

// dev address confirmed, because of the value of A0 and A1 (page 17)
#define Device_address ADC128D818_ADDRESS_LOW_LOW

void delay(int);

class ADC128D818
{
    private:
        TwoWire Wire;
        void writeTo(TwoWire Wire,uint8_t DEV_address, uint8_t reg_address, int val);
        uint8_t read_one_byte(TwoWire Wire,int DEV_address, uint8_t reg_address);
        uint16_t _read_channel_voltage_AD(TwoWire Wire,int DEV_address, uint8_t channel);


    public:
        ADC128D818();
        uint16_t read_channel(uint8_t channel);

};


#endif // ADC128D818_H