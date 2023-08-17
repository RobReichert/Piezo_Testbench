# Wire library for Red Pitaya
This library is bast on "TWI/I2C library for Arduino & Wiring Copyright (c) 2006 Nicholas Zambetti"
The most memberfunctions behave the same. 
Can be used only as I2C Master

- ``void begin();`` Open the device file to start the comunication
- ``void setClock(frequency);`` Not used in the actual version sould later be used to change the I2C Clock (standart 400kBaud)--> bootargs have to be used for setup according https://forum.redpitaya.com/viewtopic.php?t=1746
- ``void beginTransmission(address);`` initializes a connection to the slave address which is given into the function 
- ``uint8_t writeData(data);`` save 8-bit data in the write buffer, return 1 if successful and 0 if the buffer is full
- ``uint8_t writeData(data, length);`` save data array to write buffer, number of bytes needed, return number of bytes written in buffer
- ``uint8_t endTransmission(void);`` send buffer to bus and end transmission, return 0 if successful and an negativ value according ioctl() documentation if there is an error 
- ``uint8_t requestFrom(address, quantity);`` request a number (quantity) of bytes from given slave address and put it into the read buffer, return the sice of read operation output
- ``int available(void);`` return the number of bytes available for reading
- ``int readData(void);`` return the next data in read buffer or -1 if buffer is empty
- ``void flush(void);`` reset read and write buffer


