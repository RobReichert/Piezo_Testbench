#ifndef SENSOR_H
#define SENSOR_H

#include "adc128d818.h"

// declare global variables
extern float temperature_table[];
extern float resistance_table[];
extern int table_size;

class Sensor 
{
private:
    static ADC128D818 adc; // create ADC object, ADC128D818 (with i2c) will be initialized
    float _AD_to_voltage(int ad_value);
    float _voltage_to_temp_resistance(float voltage);
    float _temp_resistance_to_temperature(float resistance);

public:
    float get_channel_temperature(int channel);
    float get_channel_heat_flux(int channel);
};

#endif // SENSOR_H