# 1. IIC test with MPU6050
Aim: first, to get familiar with wire.h

## code to be tested using wire.h for IIC (draft) (get started with MPU 6050)
```c++
#include "wire.h"
#include <stdio.h>
#include <time.h>

#define MPU6050_ADDR 0xD0 //(0b1101000 <<1) + 0
//#define MPU6050_ADDR 0x68
//#define MPU6050_ADDR (0b1101000 <<1) + 1

#define SMPLRT_DIV_REG 0x19

#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41

#define PWR_MGMT_1_REG 0x6B
#define WHO_AM_I_REG 0x75
 
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

void writeTo(int DEV_address, byte reg_address, byte val)
{
    Wire.beginTransmission(DEV_address);           //start transmission to the device address
    Wire.writeData(reg_address);                      // send register address
    Wire.writeData(val);                          // send value to write
    Wire.endTransmission();                   //end transmission
}

int read_one_bit(int DEV_address, byte reg_address)
{
    int val = -1;
    Wire.beginTransmission(DEV_address);
    Wire.writeData(reg_address); // Start with ...REG
    Wire.endTransmission();
    Wire.requestFrom(DEV_address, 1); // Read 1 byte total

    val = Wire.readData();

    return val;
}

int main()
{
	int acc_x, acc_y, acc_z = 0;

	TwoWire Wire;
	Wire.begin(); // to start the IIC communication


    // Wire.beginTransmission(MPU6050_ADDR);
    // Wire.writeData(WHO_AM_I_REG); // Start with WHO_AM_I_REG
    // Wire.endTransmission();
    // Wire.requestFrom(MPU6050_ADDR, 1); // Read 1 register total

    // int check = 0;
    // check = Wire.readData();

    int check = read_one_bit(MPU6050_ADDR, WHO_AM_I_REG);

    if (check == 0x68) // hex(0x68) is dec(104)
    {
        printf("The sensor is MPU6050.\n");
    }
    else
    {
        printf("The value read from WHO_AM_I_REG:%d\n ",&check); // should be 104
        printf("The sensor is not MPU6050, or the sensor is not correctly connected!\n");
        return 0;
    }

    // initialize the MPU6050
    // power management register 0X6B we should write all 0's to wake MPU6050 up
    writeTo(MPU6050_ADDR,PWR_MGMT_1_REG,0);

    // Set DATA RATE of 1KHz by writing SMPLRT_DIV register
    writeTo(MPU6050_ADDR,SMPLRT_DIV_REG,0x07);

    // Set accelerometer configuration in ACCEL_CONFIG Register (range 2g)
    writeTo(MPU6050_ADDR,ACCEL_CONFIG_REG,0x00);

    // read from MPU6050
	for(int i=0; i<500; i++) {
		Wire.beginTransmission(MPU6050_ADDR);
		Wire.writeData(ACCEL_XOUT_H_REG); // Start with register ACCEL_XOUT_H_REG
		Wire.endTransmission();
		Wire.requestFrom(MPU6050_ADDR, 6); // Read 6 registers total, each axis value is stored in 2 registers
	
		acc_x = ( Wire.readData() | Wire.readData() << 8); //Recive Byte1 and 2 and put Byte2 on higher position and Byte1 on lower
		acc_y = ( Wire.readData() | Wire.readData() << 8); //Recive Byte3 and 4 and put Byte4 on higher position and Byte3 on lower
		acc_z = ( Wire.readData() | Wire.readData() << 8); //Recive Byte5 and 6 and put Byte6 on higher position and Byte5 on lower

		printf("acc x: %d\n",acc_x);
		
		delay(100);
		}

	return 0;
}
```


# 2. IIC with acd128d818
## plan (initialize)
* Serial Bus Address: LOW_LOW to HIGH_HIGH, `page 17`
* OPERATION_MODE (0-3)  `page 16, 24`, use *Mode 1* to use all the 8 channels?
* rate(continuous, `page 22`/one shot, `page 24`/low power, `page 22`), `page 32`
* voltage ref (internal/*external*), `page 24`
* ADC_ENABLE_TEMP (may be opotional)
* ADC_ENABLE_IN1: enable IN (input) channel
* ADC_INT_TEMP (optional): temperature interrupt, `page 29`
* enabled_mask (optional?): Interrupt Mask Register (address 03h), `page 21`

* limit reg, `page 26`?


## code to be tested using wire.h (draft)
```c++
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


void writeTo(int DEV_address, byte reg_address, byte val)
{
    Wire.beginTransmission(DEV_address);           //start transmission to the device address
    Wire.writeData(reg_address);                      // send register address
    Wire.writeData(val);                          // send value to write
    Wire.endTransmission();                   //end transmission
}

void write_Bytes_To(int DEV_address, byte reg_address, byte val, int length)
{
    Wire.beginTransmission(DEV_address);           //start transmission to the device address
    Wire.writeData(reg_address);                      // send register address
    Wire.writeData(val, length);                          // send value to write
    Wire.endTransmission();                   //end transmission
}

int read_one_bit(int DEV_address, byte reg_address)
{
    int val = -1;
    Wire.beginTransmission(DEV_address);
    Wire.writeData(reg_address); // Start with ...REG
    Wire.endTransmission();
    Wire.requestFrom(DEV_address, 1); // Read 1 byte total

    val = Wire.readData();

    return val;
}


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
    int check = read_one_bit(Device_address, ADC128D818_REG_Manufacturer_ID_Register);

    if (check !=0x01) // should be 0000 0001
    {
        printf("The device is not ADC128D818 (or not connected).\n");
        return 0;
    }

    // page 33: Quick start
    // 1. Power on the device, then wait for at least 33ms.
    // 2. Set the 'START' bit of the Configuration Register (address 00h, bit 0) to 0 (shutdown mode).
    // get in shutdown mode => bit 0 to 0 in 
    writeTo(Device_address, ADC128D818_REG_Configuration_Register, 0);

    // 3. Read the Busy Status Register (address 0Ch). If the 'Not Ready' bit = 1, then increase the wait time until 'Not Ready' bit = 0 before proceeding to the next step.
    uint8_t busy_reg = 0x00;
    int cont = 0;
    do {
            cont++;
            if (cont >= MAX_ITER_NOT_READY) 
            {

                printf("Wait for not busy ADC128D818 timeout\r\n");

                return FALSE; 
            }
            //printf("> Wait for device ready. Trial #%u\r\n", cont);

            delay(33);

            // busy: 0000 0001    not ready: 0000 0010 = 0x02
            busy_reg = read_one_bit(Device_address, ADC128D818_REG_Busy_Status_Register);

            printf(">>> Busy Status Register value: 0x%x\r\n", busy_reg);


        } while (busy_reg&( ADC128D818_STATUS_NOT_READY_BIT )); // if not ready 0x02, try again


        //    * 4. Program the Advanced Configuration Register -- Address 0Bh:
        // * - a. Choose to use the internal or external VREF (bit 0).
        // * - b. Choose the mode of operation (bits [2:1]).
        writeTo(Device_address, ADC128D818_REG_Advanced_Configuration_Register, ADC128D818_VREF_EXT | (ADC128D818_OPERATION_MODE_1 << 1));

        // 5. Program the Conversion Rate Register (address 07h).
        writeTo(Device_address, ADC128D818_REG_Conversion_Rate_Register, ADC128D818_RATE_CONTINUOUS)


        // 6. Choose to enable or disable the channels using the Channel Disable Register (address 08h).
        // enable all the channels:0b00000000 (1 means disable)
        writeTo(Device_address, ADC128D818_REG_Channel_Disable_Register, 0b00000000);


        // 7. Using the Interrupt Mask Register (address 03h), choose to mask or not to mask the interrupt status from propagating to the interrupt output pin, INT.
        // default: not masked


        // 8. Program the Limit Registers (addresses 2Ah - 39h).
        // can skip?
        writeTo(Device_address, ADC128D818_REG_Limit_Registers+ ADC128D818_CHANNEL_IN0 * 2 + ADC128D818_LIMIT_HIGH,0xFF);


        // 9. Set the 'START' bit of the Configuration Register (address 00h, bit 0) to 1.
        writeTo(Device_address, ADC128D818_REG_Configuration_Register, 1);

        // 10. Set the 'INT_Clear' bit (address 00h, bit 3) to 0. If needed, program the 'INT_Enable' bit (address 00h, bit 1) to 1 to enable the INT output.
        // already done in step 9?




    return 0;
}

```



# 3. backup
## earlier code in STM32 system (IIC-MPU6050)
```c
#define MPU6050_ADDR 0xD0 //(0b1101000 <<1) + 0
//#define MPU6050_ADDR 0x68
//#define MPU6050_ADDR (0b1101000 <<1) + 1

#define SMPLRT_DIV_REG 0x19

#define ACCEL_CONFIG_REG 0x1C
#define ACCEL_XOUT_H_REG 0x3B
#define TEMP_OUT_H_REG 0x41

#define PWR_MGMT_1_REG 0x6B
#define WHO_AM_I_REG 0x75

void MPU6050_Init (void)//(MPU6050_t *imu1)
{
	uint8_t check;
	uint8_t Data;
	char str[50];

	// check device ID WHO_AM_I
    //hi2c Pointer, 
    //DevAddress, 
    //Internal memory address, 
    //Size of internal memory address,
    //Pointer to data buffer,
    //Amount of data to be sent
    //Timeout duration
	HAL_I2C_Mem_Read (&hi2c2, MPU6050_ADDR,WHO_AM_I_REG,1, &check, 1, 1000);

	if (check == 0x68)  // 0x68 will be returned by the sensor if everything goes well
	{
		// power management register 0X6B we should write all 0's to wake the sensor up
		Data = 0;
		HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, PWR_MGMT_1_REG, 1,&Data, 1, 1000);

		// Set DATA RATE of 1KHz by writing SMPLRT_DIV register
		Data = 0x07;
		HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, SMPLRT_DIV_REG, 1, &Data, 1, 1000);

		// Set accelerometer configuration in ACCEL_CONFIG Register
		// XA_ST=0,YA_ST=0,ZA_ST=0, FS_SEL=0 -> �??????? 2g
		Data = 0x00;
		HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &Data, 1, 1000);
	}
	else
	{
		sprintf(str, "The sensor is not MPU6050\r\n");
		HAL_UART_Transmit(&huart1,(uint8_t*)str,strlen(str),0XFFFF);
		HAL_Delay(10);
	}

}

void MPU6050_Read_Accel (MPU6050_t *imu1)
{
	uint8_t Rec_Data[6];

	// Read 6 BYTES of data starting from ACCEL_XOUT_H register

	HAL_I2C_Mem_Read (&hi2c2, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, Rec_Data, 6, 1000);

	imu1->Accel_X_RAW = (int16_t)(Rec_Data[0] << 8 | Rec_Data [1]);
	imu1->Accel_Y_RAW = (int16_t)(Rec_Data[2] << 8 | Rec_Data [3]);
	imu1->Accel_Z_RAW = (int16_t)(Rec_Data[4] << 8 | Rec_Data [5]);

	/*** convert the RAW values into acceleration in 'g'
	     we have to divide according to the Full scale value set in FS_SEL
	     I have configured FS_SEL = 0. So I am dividing by 16384.0
	     for more details check ACCEL_CONFIG Register              ****/

	imu1->Ax = imu1->Accel_X_RAW/16384.0;
	imu1->Ay = imu1->Accel_Y_RAW/16384.0;
	imu1->Az = imu1->Accel_Z_RAW/16384.0;
}
```