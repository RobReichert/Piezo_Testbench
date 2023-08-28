#include "wire.h"
#include <stdio.h>
#include <time.h>

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

// dev address still to be confirmed, because of the value of A0 and A1 (page 17)
#define Device_address ADC128D818_ADDRESS_LOW_LOW


void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

void writeTo(TwoWire Wire,int DEV_address, int reg_address, int val)
{
    Wire.beginTransmission(DEV_address);           //start transmission to the device address
    Wire.writeData(reg_address);                      // send register address
    Wire.writeData(val);                          // send value to write
    Wire.endTransmission();                   //end transmission
}

int read_one_bit(TwoWire Wire,int DEV_address, int reg_address)
{
    int val = -1;
    Wire.beginTransmission(DEV_address);
    Wire.writeData(reg_address); // Start with ...REG
    Wire.endTransmission();
    Wire.requestFrom(DEV_address, 1); // Read 1 byte total

    val = Wire.readData();

    return val;
}

/*
void write_Bytes_To(TwoWire Wire,int DEV_address, int reg_address, int val, int length)
{
    Wire.beginTransmission(DEV_address);           //start transmission to the device address
    Wire.writeData(reg_address);                      // send register address
    Wire.writeData(val, length);                          // send value to write
    Wire.endTransmission();                   //end transmission
}
*/



int main(void)
{
    TwoWire Wire;
	Wire.begin(); // to start the IIC communication

    //adc128d818_init

    // Wire.beginTransmission(ADC128D818_ADDRESS_LOW_LOW);
    // Wire.writeData(ADC128D818_REG_Manufacturer_ID_Register); // Start with ADC128D818_REG_Manufacturer_ID_Register
    // Wire.endTransmission();
    // Wire.requestFrom(ADC128D818_ADDRESS_LOW_LOW, 1); // Read 1 register total

    // test alive: read the Manufacturer ID register (must be != 0)
    int check = read_one_bit(Wire, Device_address, ADC128D818_REG_Manufacturer_ID_Register);

    if (check !=0x01) // should be 0000 0001
    {
        printf("The device is not ADC128D818 (or not connected).\n");
        return 0;
    }

    // page 33: Quick start
    // 1. Power on the device, then wait for at least 33ms.
    // 2. Set the 'START' bit of the Configuration Register (address 00h, bit 0) to 0 (shutdown mode).
    // get in shutdown mode => bit 0 to 0 in 
    writeTo(Wire, Device_address, ADC128D818_REG_Configuration_Register, 0);

    // 3. Read the Busy Status Register (address 0Ch). If the 'Not Ready' bit = 1, then increase the wait time until 'Not Ready' bit = 0 before proceeding to the next step.
    uint8_t busy_reg = 0x00;
    int cont = 0;
    do {
            cont++;
            if (cont >= MAX_ITER_NOT_READY) 
            {

                printf("Wait for not busy ADC128D818 timeout\r\n");

                return 0; 
            }
            //printf("> Wait for device ready. Trial #%u\r\n", cont);

            delay(33);

            // busy: 0000 0001    not ready: 0000 0010 = 0x02
            busy_reg = read_one_bit(Wire, Device_address, ADC128D818_REG_Busy_Status_Register);

            printf(">>> Busy Status Register value: 0x%x\r\n", busy_reg);


        } while (busy_reg&( ADC128D818_STATUS_NOT_READY_BIT )); // if not ready 0x02, try again


        //    * 4. Program the Advanced Configuration Register -- Address 0Bh:
        // * - a. Choose to use the internal or external VREF (bit 0).
        // * - b. Choose the mode of operation (bits [2:1]).
        writeTo(Wire, Device_address, ADC128D818_REG_Advanced_Configuration_Register, ADC128D818_VREF_EXT | (ADC128D818_OPERATION_MODE_1 << 1));

        // 5. Program the Conversion Rate Register (address 07h).
        //ADC128D818_RATE_CONTINUOUS 0b00000001
        writeTo(Wire, Device_address, ADC128D818_REG_Conversion_Rate_Register, 0b00000001);


        // 6. Choose to enable or disable the channels using the Channel Disable Register (address 08h).
        // enable all the channels:0b00000000 (1 means disable)
        writeTo(Wire, Device_address, ADC128D818_REG_Channel_Disable_Register, 0b00000000);


        // 7. Using the Interrupt Mask Register (address 03h), choose to mask or not to mask the interrupt status from propagating to the interrupt output pin, INT.
        // default: not masked


        // 8. Program the Limit Registers (addresses 2Ah - 39h).
        // can skip?
        writeTo(Wire, Device_address, ADC128D818_REG_Limit_Registers+ ADC128D818_CHANNEL_IN0 * 2 + ADC128D818_LIMIT_HIGH,0xFF);


        // 9. Set the 'START' bit of the Configuration Register (address 00h, bit 0) to 1.
        writeTo(Wire, Device_address, ADC128D818_REG_Configuration_Register, 1);

        // 10. Set the 'INT_Clear' bit (address 00h, bit 3) to 0. If needed, program the 'INT_Enable' bit (address 00h, bit 1) to 1 to enable the INT output.
        // already done in step 9?




    return 0;
}