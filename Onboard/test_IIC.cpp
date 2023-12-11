#include "wire.h"
#include <stdio.h>
#include <time.h>

//#define MPU6050_ADDR 0xD0 //(0b1101000 <<1) + 0
#define MPU6050_ADDR 0x68
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

    int check = read_one_bit(Wire,MPU6050_ADDR, WHO_AM_I_REG);
    
/*     Wire.beginTransmission(0x68);
    Wire.writeData(0x75); // Start with ...REG
    Wire.endTransmission();
    Wire.requestFrom(0x68, 1); // Read 1 byte total

    int check = Wire.readData(); */

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
    writeTo(Wire,MPU6050_ADDR,PWR_MGMT_1_REG,0);

    // Set DATA RATE of 1KHz by writing SMPLRT_DIV register
    writeTo(Wire,MPU6050_ADDR,SMPLRT_DIV_REG,0x07);

    // Set accelerometer configuration in ACCEL_CONFIG Register (range 2g)
    writeTo(Wire,MPU6050_ADDR,ACCEL_CONFIG_REG,0x00);

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