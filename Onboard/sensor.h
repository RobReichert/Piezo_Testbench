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
    float _temp_resistance_to_temperature_lookup_table(float resistance);
    float _temp_resistance_to_temperature_calibration(float resistance, int R_0, float B);
    float _temp_resistance_to_temperature_Steinhart(float resistance, float A, float B, float C);

public:
    float get_channel_temperature_lookup_table(int channel);            // get temperature from lookup table in data sheet
    float get_channel_temp_calibration(int channel, int R_0, float B); // get temperature from Beta parameter equation

    float get_channel_temp_Steinhart(int channel, float A, float B, float C); // get temperature from Steinhart-Hart equation

    float get_channel_heat_flux(int channel);

    float get_channel_R(int channel);
};

#endif // SENSOR_H