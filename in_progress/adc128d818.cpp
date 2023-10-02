#include "adc128d818.h"
#include <stdio.h>
#include <stdexcept>


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

void ADC128D818::writeTo(TwoWire Wire,uint8_t DEV_address, uint8_t reg_address, int val)
{
    Wire.beginTransmission(DEV_address);           //start transmission to the device address
    Wire.writeData(reg_address);                   // send register address
    Wire.writeData(val);                           // send value to write
    Wire.endTransmission();                        //end transmission
}

uint8_t ADC128D818::read_one_byte(TwoWire Wire,int DEV_address, uint8_t reg_address)
{
    uint8_t val = -1;
    Wire.beginTransmission(DEV_address);
    Wire.writeData(reg_address);        // Start with ...REG
    Wire.endTransmission();
    Wire.requestFrom(DEV_address, 1);   // Read 1 byte total

    val = Wire.readData();

    return val;
}


uint16_t ADC128D818::_read_channel_voltage_AD(TwoWire Wire,int DEV_address, uint8_t channel)
{
    uint8_t channel_adr;
    uint16_t result;

    if (channel>7 || channel<0)
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

/// @brief Read the 12 bit AD convertion result of ADC128D818 from the specific channel
/// @param channel the channel number (0~7)
/// @return the 12-bits ADC value
uint16_t ADC128D818::read_channel(uint8_t channel)
{
    return _read_channel_voltage_AD(this->Wire,Device_address,channel);
}



ADC128D818::ADC128D818() // Constructor
{
    //adc128d818_init

    
	this->Wire.begin(); // to start the IIC communication

    // test alive: read the Manufacturer ID register (should be 0x01)
    uint8_t check = read_one_byte(this->Wire, Device_address, ADC128D818_REG_Manufacturer_ID_Register);

    if (check !=0x01) // should be 0000 0001
    {
        printf("The device is not ADC128D818 (or not connected).\n");
        printf("Manufacturer_ID:%d\n", check);

        throw std::runtime_error("Fail to connect to ADC128D818");


        //return false;
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
    writeTo(this->Wire, Device_address, ADC128D818_REG_Configuration_Register, 0);

    // 3. Read the Busy Status Register (address 0Ch). If the 'Not Ready' bit = 1, then increase the wait time until 'Not Ready' bit = 0 before proceeding to the next step.
    uint8_t busy_reg = 0x00;
    int cont = 0;
    do {
            cont++;
            if (cont >= MAX_ITER_NOT_READY) 
            {

                printf("Wait for not busy ADC128D818 timeout\r\n");

                throw std::runtime_error("Wait for not busy ADC128D818 timeout");
                //return false; 
            }
            //printf("> Wait for device ready. Trial #%u\r\n", cont);

            delay(33);

            // busy: 0000 0001    not ready: 0000 0010 = 0x02
            // not busy and ready: 0x00
            busy_reg = read_one_byte(this->Wire, Device_address, ADC128D818_REG_Busy_Status_Register);

            printf(">>> Busy Status Register value: 0x%x\r\n", busy_reg);


        } while (busy_reg&( ADC128D818_STATUS_NOT_READY_BIT )); // if not ready 0x02, try again


    //    * 4. Program the Advanced Configuration Register -- Address 0Bh:
    // * - a. Choose to use the internal or external VREF (bit 0).
    // * - b. Choose the mode of operation (bits [2:1]).
    writeTo(this->Wire, Device_address, ADC128D818_REG_Advanced_Configuration_Register, ADC128D818_VREF_EXT | (ADC128D818_OPERATION_MODE_1 << 1));

    // 5. Program the Conversion Rate Register (address 07h).
    //ADC128D818_RATE_CONTINUOUS 0b00000001
    writeTo(this->Wire, Device_address, ADC128D818_REG_Conversion_Rate_Register, 0b00000001);


    // 6. Choose to enable or disable the channels using the Channel Disable Register (address 08h).
    // enable all the channels:0b00000000 (1 means disable)
    //writeTo(Wire, Device_address, ADC128D818_REG_Channel_Disable_Register, 0b00000000);
    writeTo(this->Wire, Device_address, ADC128D818_REG_Channel_Disable_Register, 0x00);

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
    writeTo(this->Wire, Device_address, ADC128D818_REG_Limit_Registers+ ADC128D818_CHANNEL_IN0 * 2 + ADC128D818_LIMIT_HIGH,0xFF);


    // 9. Set the 'START' bit of the Configuration Register (address 00h, bit 0) to 1.
    writeTo(this->Wire, Device_address, ADC128D818_REG_Configuration_Register, 1);

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

    //return true;
}
