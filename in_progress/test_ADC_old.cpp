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


void delay(int number_of_mili_seconds)
{
    // Converting time into milli_seconds
    int micro_seconds = 1000 * number_of_mili_seconds;
 
    // Storing start time
    clock_t start_time = clock();
 
    // looping till required time is not achieved
    while (clock() < start_time + micro_seconds)
        ;
}

/// @brief Writes a value to a specified register address of a device using the TwoWire protocol.
/// @param Wire The TwoWire object used for communication.
/// @param DEV_address The address of the device.
/// @param reg_address The register address to write to.
/// @param val The value to be written.
void writeTo(TwoWire Wire,uint8_t DEV_address, uint8_t reg_address, int val)
{
    Wire.beginTransmission(DEV_address);           //start transmission to the device address
    Wire.writeData(reg_address);                   // send register address
    Wire.writeData(val);                           // send value to write
    Wire.endTransmission();                        //end transmission
}

/// @brief Reads one byte of data from a specified register address of a device using the TwoWire protocol.
/// @param Wire The TwoWire object used for communication.
/// @param DEV_address The address of the device.
/// @param reg_address The register address to read from.
/// @return The value read from the specified register address.
uint8_t read_one_byte(TwoWire Wire,int DEV_address, uint8_t reg_address)
{
    uint8_t val = -1;
    Wire.beginTransmission(DEV_address);
    Wire.writeData(reg_address);        // Start with ...REG
    Wire.endTransmission();
    Wire.requestFrom(DEV_address, 1);   // Read 1 byte total

    val = Wire.readData();

    return val;
}


/// @brief Read the 12 bit AD convertion result of ADC128D818 from the specific channel using the TwoWire protocol.
/// @param Wire The TwoWire object used for communication.
/// @param DEV_address The address of the device (ADC128D818).
/// @param channel The channel number of the ADC128D818 to read from
/// @return The AD value (12 bit) read from the channel
static uint16_t _read_channel_voltage_AD(TwoWire Wire,int DEV_address, uint8_t channel)
{
    uint8_t channel_adr;
    uint16_t result;

    if (channel>7)
    {
        printf("channel should be 0-7.\n");
        return 0;
    }

    // the register for this chanel
    channel_adr = ADC128D818_REG_Channel_Readings_Registers + channel;
    #if DEBUG==1
        printf("channel: 0x%x\n",channel_adr);
    #endif

    // test shutdown mode
    //TODO, or can skip
    writeTo(Wire, Device_address, ADC128D818_REG_Configuration_Register, 1);

    uint8_t config_reg_val=read_one_byte(Wire, Device_address, ADC128D818_REG_Configuration_Register);
    if (config_reg_val==0)
    {
        printf("shutdown mode");
        return 0;
    }
    else
    {
        #if DEBUG==1
            printf("config_reg_val: 0x%02x\n",config_reg_val);
        #endif
    }



    Wire.beginTransmission(DEV_address);
	Wire.writeData(channel_adr); // Start with the register of this channel
	Wire.endTransmission();
	Wire.requestFrom(DEV_address, 2); // Read 2 bytes totally

    //result = ( Wire.readData() | Wire.readData() << 8); //TODO: to be confirmed
    result = ( Wire.readData() << 8 | Wire.readData() );
    #if DEBUG==1
        printf("result 16 bit: 0x%02x\n",result);
    #endif

    return (result >> 4); // 12 bits for voltage
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

uint16_t read_channel_ADC128D818(TwoWire Wire, uint8_t channel)
{
    return _read_channel_voltage_AD(Wire,Device_address,channel);
}

bool init_ADC128D818(TwoWire Wire)
{
    //adc128d818_init

    // test alive: read the Manufacturer ID register (should be 0x01)
    uint8_t check = read_one_byte(Wire, Device_address, ADC128D818_REG_Manufacturer_ID_Register);

    if (check !=0x01) // should be 0000 0001
    {
        printf("The device is not ADC128D818 (or not connected).\n");
        printf("Manufacturer_ID:%d\n", check);
        return false;
    }
    else
    {
    #if Debug==1
        printf("The device is ADC128D818!.\n");
    #endif
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

                return false; 
            }
            //printf("> Wait for device ready. Trial #%u\r\n", cont);

            delay(33);

            // busy: 0000 0001    not ready: 0000 0010 = 0x02
            // not busy and ready: 0x00
            busy_reg = read_one_byte(Wire, Device_address, ADC128D818_REG_Busy_Status_Register);

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
    //writeTo(Wire, Device_address, ADC128D818_REG_Channel_Disable_Register, 0b00000000);
    writeTo(Wire, Device_address, ADC128D818_REG_Channel_Disable_Register, 0x00);

#if Debug==1
    printf("Will read channel disable status.\n\n");    
    uint8_t channel_enable_status = read_one_byte(Wire, Device_address, ADC128D818_REG_Channel_Disable_Register);
    


    if (channel_enable_status == 0b00000000)
    {
        // should be 0000 0000
        printf("channel_enable_status: 0x%02x\n", channel_enable_status); 
        printf("Channel enable 1st time: successful!\n\n");
    }
    else
    {
        printf("channel_enable_status 1st time: 0x%02x\n", channel_enable_status);
        printf("Channel enable 1st time: not successful?\n");
    }
#endif
    


    // 7. Using the Interrupt Mask Register (address 03h), choose to mask or not to mask the interrupt status from propagating to the interrupt output pin, INT.
    // default: not masked


    // 8. Program the Limit Registers (addresses 2Ah - 39h).
    // can skip?
    writeTo(Wire, Device_address, ADC128D818_REG_Limit_Registers+ ADC128D818_CHANNEL_IN0 * 2 + ADC128D818_LIMIT_HIGH,0xFF);


    // 9. Set the 'START' bit of the Configuration Register (address 00h, bit 0) to 1.
    writeTo(Wire, Device_address, ADC128D818_REG_Configuration_Register, 1);

    // 10. Set the 'INT_Clear' bit (address 00h, bit 3) to 0. If needed, program the 'INT_Enable' bit (address 00h, bit 1) to 1 to enable the INT output.
    // already done in step 9?

    #if Debug==1
        printf("To the end of the configuration\n\n");
    #endif

    // cheak config
#if Debug==1
    channel_enable_status = read_one_byte(Wire, Device_address, ADC128D818_REG_Channel_Disable_Register);


    if (channel_enable_status == 0b00000000)
    {
        // should be 0000 0000
        printf("channel_enable_status: %d\n", channel_enable_status); 
        printf("Channel enable 2nd time: successful!\n\n");
    }
    else
    {
        printf("channel_enable_status 2nd time: %d\n", channel_enable_status);
        printf("Channel enable 2nd time: not successful?\n");
    }
#endif

    delay(100); // it is necessary to wait here before going further

    return true;
}




int main(void)
{
    TwoWire Wire;
	Wire.begin(); // to start the IIC communication

    bool init_status = init_ADC128D818(Wire);
    if (init_status != true)
    {
        printf("Fail to initialize ADC128D818.");
        return 0;
    }


    // read channel value
    
    uint16_t val_channel_0;
    uint16_t val_channel_1;
    uint16_t val_channel_2;
    uint16_t val_channel_3;
    uint16_t val_channel_4;
    uint16_t val_channel_5;
    uint16_t val_channel_6;
    uint16_t val_channel_7;

    for(int i=1;i<2;i++)
    {
        val_channel_0 = read_channel_ADC128D818(Wire, 0);
        printf("(12 bit) channel value 0: %d\n\n", val_channel_0);
        

        val_channel_1 = read_channel_ADC128D818(Wire, 1);
        printf("(12 bit) channel value 1: %d\n\n", val_channel_1);


        val_channel_2 = read_channel_ADC128D818(Wire, 2);
        printf("(12 bit) channel value 2: %d\n\n", val_channel_2);


        val_channel_3 = read_channel_ADC128D818(Wire, 3);
        printf("(12 bit) channel value 3: %d\n\n", val_channel_3);


        val_channel_4 = read_channel_ADC128D818(Wire, 4);
        printf("(12 bit) channel value 4: %d\n\n", val_channel_4);
        

        val_channel_5 = read_channel_ADC128D818(Wire, 5);
        printf("(12 bit) channel value 5: %d\n\n", val_channel_5);


        val_channel_6 = read_channel_ADC128D818(Wire, 6);
        printf("(12 bit) channel value 6: %d\n\n", val_channel_6);


        val_channel_7 = read_channel_ADC128D818(Wire, 7);
        printf("(12 bit) channel value 7: %d\n\n", val_channel_7);

        //delay(1000);
    }




    //printf("new complie\n");

    return 0;
}