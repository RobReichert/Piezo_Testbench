#include "wire.h"
#include <stdio.h>
#include <time.h>
 
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


int main()
{
	int acc_x, acc_y, acc_z = 0;

	TwoWire Wire;
	Wire.begin();
	Wire.beginTransmission(0x53);
	Wire.writeData(0x2D);
	Wire.writeData(0b00001000);
	Wire.endTransmission();
	for(int i=0; i<500; i++) {
	Wire.beginTransmission(0x53);
  	Wire.writeData(0x32); // Start with register 0x32 (ACCEL_XOUT_H)
  	Wire.endTransmission();
  	Wire.requestFrom(0x53, 6); // Read 6 registers total, each axis value is stored in 2 registers
 
	acc_x = ( Wire.readData() | Wire.readData() << 8); //Recive Byte1 and 2 and put Byte2 on higher position and Byte1 on lower
  	acc_y = ( Wire.readData() | Wire.readData() << 8); //Recive Byte3 and 4 and put Byte4 on higher position and Byte3 on lower
  	acc_z = ( Wire.readData() | Wire.readData() << 8); //Recive Byte5 and 6 and put Byte6 on higher position and Byte5 on lower

	printf("acc x: %d\n",acc_x);
	
	delay(100);
	}

	return 0;
}