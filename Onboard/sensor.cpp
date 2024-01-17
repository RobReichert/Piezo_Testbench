#include "sensor.h"
#include <iostream> // Used for using std::cerr to output error messages
#include <math.h> // Used for log() function

// declaration and definition of temperature_table and resistance_table (global variables)
// data sheet of NRG1104F3380B1F
float temperature_table[] = {
    -40.0, -39.0, -38.0, -37.0, -36.0,
    -35.0, -34.0, -33.0, -32.0, -31.0,
    -30.0, -29.0, -28.0, -27.0, -26.0,
    -25.0, -24.0, -23.0, -22.0, -21.0,
    -20.0, -19.0, -18.0, -17.0, -16.0,
    -15.0, -14.0, -13.0, -12.0, -11.0,
    -10.0, -9.0, -8.0, -7.0, -6.0,
    -5.0, -4.0, -3.0, -2.0, -1.0,
    0.0,
    1.0, 2.0, 3.0, 4.0, 5.0,
    6.0, 7.0, 8.0, 9.0, 10.0,
    11.0, 12.0, 13.0, 14.0, 15.0,
    16.0, 17.0, 18.0, 19.0, 20.0,
    21.0, 22.0, 23.0, 24.0, 25.0,
    26.0, 27.0, 28.0, 29.0, 30.0,
    31.0, 32.0, 33.0, 34.0, 35.0,
    36.0, 37.0, 38.0, 39.0, 40.0,
    41.0, 42.0, 43.0, 44.0, 45.0,
    46.0, 47.0, 48.0, 49.0, 50.0,
    51.0, 52.0, 53.0, 54.0, 55.0,
    56.0, 57.0, 58.0, 59.0, 60.0,
    61.0, 62.0, 63.0, 64.0, 65.0,
    66.0, 67.0, 68.0, 69.0, 70.0,
    71.0, 72.0, 73.0, 74.0, 75.0,
    76.0, 77.0, 78.0, 79.0, 80.0,
    81.0, 82.0, 83.0, 84.0, 85.0,
    86.0, 87.0, 88.0, 89.0, 90.0,
    91.0, 92.0, 93.0, 94.0, 95.0,
    96.0, 97.0, 98.0, 99.0, 100.0,
    101.0, 102.0, 103.0, 104.0, 105.0};

float resistance_table[] = {
    195000.0, 184630.0, 174870.0, 165670.0, 157010.0, // -40.0, -39.0, -38.0, -37.0, -36.0,
    148840.0, 141140.0, 133890.0, 127040.0, 120580.0, // -35.0, -34.0, -33.0, -32.0, -31.0,
    114480.0, 108730.0, 103290.0, 98150.0, 93300.0, // -30.0, -29.0, -28.0, -27.0, -26.0,
    88710.0, 84380.0, 80280.0, 76390.0, 72720.0, // -25.0, -24.0, -23.0, -22.0, -21.0,
    69250.0, 65960.0, 62840.0, 59880.0, 57090.0, // -20.0, -19.0, -18.0, -17.0, -16.0,
    54430.0, 51920.0, 49530.0, 47270.0, 45120.0, // -15.0, -14.0, -13.0, -12.0, -11.0,
    43080.0, 41150.0, 39310.0, 37560.0, 35900.0, // -10.0, -9.0, -8.0, -7.0, -6.0,
    34330.0, 32830.0, 31400.0, 30050.0, 28760.0, // -5.0, -4.0, -3.0, -2.0, -1.0,
    27530.0, // 0.0
    26360.0, 25250.0, 24190.0, 23180.0, 22220.0, // 1.0, 2.0, 3.0, 4.0, 5.0,
    21300.0, 20430.0, 19590.0, 18800.0, 18040.0, // 6.0, 7.0, 8.0, 9.0, 10.0,
    17320.0, 16630.0, 15970.0, 15340.0, 14730.0, // 11.0, 12.0, 13.0, 14.0, 15.0,
    14160.0, 13610.0, 13090.0, 12580.0, 12100.0, // 16.0, 17.0, 18.0, 19.0, 20.0,
    11650.0, 11210.0, 10790.0, 10830.0, 10000.0, // 21.0, 22.0, 23.0, 24.0, 25.0,
    9630.0, 9280.0, 8940.0, 8620.0, 8310.0, // 26.0, 27.0, 28.0, 29.0, 30.0,
    8010.0, 7720.0, 7450.0, 7190.0, 6940.0, // 31.0, 32.0, 33.0, 34.0, 35.0,
    6690.0, 6460.0, 6240.0, 6030.0, 5820.0, // 36.0, 37.0, 38.0, 39.0, 40.0,
    5620.0, 5430.0, 5250.0, 5050.0, 4910.0, // 41.0, 42.0, 43.0, 44.0, 45.0,
    4750.0, 4590.0, 4440.0, 4300.0, 4160.0, // 46.0, 47.0, 48.0, 49.0, 50.0,
    4030.0, 3900.0, 3780.0, 3660.0, 3540.0, // 51.0, 52.0, 53.0, 54.0, 55.0,
    3430.0, 3330.0, 3220.0, 3120.0, 3030.0, // 56.0, 57.0, 58.0, 59.0, 60.0,
    2940.0, 2850.0, 2760.0, 2680.0, 2600.0, // 61.0, 62.0, 63.0, 64.0, 65.0,
    2520.0, 2450.0, 2380.0, 2310.0, 2240.0, // 66.0, 67.0, 68.0, 69.0, 70.0,
    2180.0, 2120.0, 2060.0, 2000.0, 1940.0, // 71.0, 72.0, 73.0, 74.0, 75.0,
    1890.0, 1840.0, 1790.0, 1740.0, 1690.0, // 76.0, 77.0, 78.0, 79.0, 80.0,
    1640.0, 1600.0, 1560.0, 1510.0, 1470.0, // 81.0, 82.0, 83.0, 84.0, 85.0,
    1440.0, 1400.0, 1360.0, 1330.0, 1290.0, // 86.0, 87.0, 88.0, 89.0, 90.0,
    1260.0, 1230.0, 1190.0, 1160.0, 1140.0, // 91.0, 92.0, 93.0, 94.0, 95.0,
    1110.0, 1080.0, 1050.0, 1030.0, 1000.0, // 96.0, 97.0, 98.0, 99.0, 100.0,
    977.0, 953.3, 930.3, 907.9, 886.3}; // 101.0, 102.0, 103.0, 104.0, 105.0

// Linear interpolation to calculate temperature
int table_size = sizeof(temperature_table) / sizeof(temperature_table[0]);


// define the static member variable of the class//////////////////////////////
ADC128D818 Sensor::adc; 


// Private Methods //////////////////////////////////////////////////////////////

float Sensor::_AD_to_voltage(int ad_value) 
{
    // ADC128D818 data sheet page 28
    float V_ref = 5.0;
    float voltage = (ad_value * V_ref) / 4095.0;
    return voltage;
}

float Sensor::_voltage_to_temp_resistance(float voltage) 
{
    // series resistance: 3.9K ohm
    // GND - Temperature sensor - series resistance - 5V
    float R_series = 3900.0;
    float R_thermistor = (voltage * R_series) / (5.0 - voltage);
    return R_thermistor;
}

float Sensor::_temp_resistance_to_temperature_lookup_table(float resistance) 
{

    // Loop through the temperature points in the lookup table
    for (int i = 1; i < table_size; i++)
    {
        // Find the two temperature points closest to the given resistance value
        if (resistance >= resistance_table[i])
        {
            // Use linear interpolation to calculate the temperature
            float resistance_diff = resistance_table[i - 1] - resistance_table[i];
            float temperature_diff = temperature_table[i - 1] - temperature_table[i];
            float temperature = temperature_table[i] + (temperature_diff / resistance_diff) * (resistance - resistance_table[i]);
            return temperature;
        }
    }
        
}

// Function to convert resistance to temperature using Beta parameter equation
float Sensor::_temp_resistance_to_temperature_calibration(float resistance, int R_0, float B) 
{
    float T_0 = 298.15; // Reference temperature (T_0) in Kelvin, which is 25°C
    float temperature;

    // Convert resistance to temperature using Beta parameter equation
    temperature = (1.0 / (1.0 / T_0 + (1.0 / B) * log(resistance / R_0)));

    // Convert temperature from Kelvin to Celsius
    temperature -= 273.15;

    return temperature;
}

float Sensor::_temp_resistance_to_temperature_Steinhart(float resistance, float A, float B, float C) 
{
    // Check for valid resistance value (greater than zero)
    if (resistance <= 0.0f) {
        return -273.15f; // Return absolute zero as an indicator of invalid input
    }

    // Calculate the reciprocal of the temperature using Steinhart-Hart equation
    float invT = A + B * log(resistance) + C * pow(log(resistance), 3);

    // Convert temperature from Kelvin to Celsius
    float temperatureC = (1.0f / invT) - 273.15f;

    return temperatureC;
}




// Public Methods //////////////////////////////////////////////////////////////

/// @brief Get the temperature of a given channel (using lookup table in the data sheet)
/// @param channel The channel number of temperature sensor (0-5)
/// @return the temperature in degree Celsius (-40 - 105 degree Celsius)
float Sensor::get_channel_temperature_lookup_table(int channel) 
{
    // Ensure channel is within a valid range
    if (channel < 0 || channel > 5) {
        std::cerr << "Error: Invalid channel value for temperature. Please provide a value between 0 and 5." << std::endl;
        // return a special value -100.0 to indicate an error
        return -999.0;
    }

    int ad_value = this->adc.read_channel(channel);
    float voltage = _AD_to_voltage(ad_value);
    float resistance = _voltage_to_temp_resistance(voltage);

    if (resistance >= resistance_table[0])
    {
        // check whether the thermistor is connected or not
        // If the thermistor is not connected, the resistance (GND - temp) will be infinitely large.
        printf("Thermistor is not connected on channel %d\n", channel);
        return -999.0;
    }

    if (resistance < resistance_table[table_size - 1])
    {   // temperature too high (>105 degree celsius), out of range
        // If the given resistance value is outside the range of the lookup table, return a default value or handle the error
        // printf("Error: Resistance value outside lookup table range.\n");
        printf("Temperature on channel %d is too high and out of range(>105 degree celsius).\n", channel);
        return -999; // Default value to indicate an error
    }

    float temperature = _temp_resistance_to_temperature_lookup_table(resistance);
    return temperature;
}

/// @brief Get the temperature of a given channel (using Beta parameter equation) (calibration range: 20 - 105 degree Celsius)
/// @param channel The channel number of temperature sensor (0-5)
/// @param R_0 (int) The resistance of the thermistor at 25 degree celsius in ohm
/// @param B (float) The Beta parameter of the thermistor (unit: Kelvin)
/// @return the temperature in degree Celsius
float Sensor::get_channel_temp_calibration(int channel, int R_0, float B)
{
    // Ensure channel is within a valid range
    if (channel < 0 || channel > 5) {
        std::cerr << "Error: Invalid channel value for temperature. Please provide a value between 0 and 5." << std::endl;
        // return a special value -100.0 to indicate an error
        return -999.0;
    }

    int ad_value = this->adc.read_channel(channel);
    float voltage = _AD_to_voltage(ad_value);
    float resistance = _voltage_to_temp_resistance(voltage);

    float temperature = _temp_resistance_to_temperature_calibration(resistance, R_0, B);

    return temperature;
}

float Sensor::get_channel_temp_Steinhart(int channel, float A, float B, float C)
{
    // Ensure channel is within a valid range
    if (channel < 0 || channel > 5) {
        std::cerr << "Error: Invalid channel value for temperature. Please provide a value between 0 and 5." << std::endl;
        // return a special value -100.0 to indicate an error
        return -999.0;
    }

    int ad_value = this->adc.read_channel(channel);
    float voltage = _AD_to_voltage(ad_value);
    float resistance = _voltage_to_temp_resistance(voltage);

    float temperature = _temp_resistance_to_temperature_Steinhart(resistance, A, B, C);

    return temperature;
}


/// @brief Get the heat flux of a given channel
/// @param channel The channel number of heat flux sensor (6-7)
/// @param temperature The temperature of the heat flux sensor in degree Celsius
/// @param V_out_0 The output voltage of the heat flux sensor at 0 W/m^2 (default: 2.5)
/// @param V_gain the gain of the amplifier (unit: V/V). channel voltage = V_gain * heat_flux_sensor_voltage + V_out_0
/// @return the heat flux in W/m^2
float Sensor::get_channel_heat_flux(int channel, float temperature, float V_gain, float V_out_0) {
    // Constants
    const float S_calib = 1.32e-6; // Sensitivity at 25°C in V/(W/m^2)
    const float max_heat_flux = 150000; // Maximum heat flux in W/m^2
    const float min_heat_flux = -150000; // Minimum heat flux in W/m^2

        
    const float temperature_coefficient = 0.00334; // Temperature coefficient of heat flux sensor to adjust sensitivity S_Tc



    // Check if the channel is within the valid range
    if (channel < 6 || channel > 7) {
        std::cerr << "Error: Invalid channel value for heat flux. Please provide a value between 6 and 7." << std::endl;
        return -999.0;
    }

    // Check if the temperature of the heat flux sensor is within the valid range in the data sheet
    if (temperature < -50 || temperature > 120) {
        std::cerr << "Error: Invalid temperature value for heat flux. Please provide a value between -50 and 120." << std::endl;
        return -999.0;
    }

    // Read the ADC value from the specified channel
    int ad_value = this->adc.read_channel(channel);
    // Convert ADC value to voltage
    float voltage = _AD_to_voltage(ad_value);

    // Calculate the sensitivity at the current temperature
    float S_Tc = (temperature_coefficient * temperature + 0.917) * S_calib; // Sensitivity at current temperature

    // Convert voltage to heat flux
    float heat_flux = ((voltage - V_out_0)/V_gain) / S_Tc;



    // Check if the heat flux is within the valid range
    if (heat_flux > max_heat_flux || heat_flux < min_heat_flux) {
        std::cout << "Warning: Heat flux out of range: " << heat_flux << " W/m^2" << std::endl;
    }

    // printf("Heat flux on channel %d is %f W/m^2\n", channel, heat_flux);

    return heat_flux;
}

/// @brief Get the resistance of a given channel (notice: the series resistance is 3.9K ohm)
/// @param channel The channel number of temperature sensor (0-5)
/// @return the resistance in ohm
float Sensor::get_channel_R(int channel)
{
    // Ensure channel is within a valid range
    if (channel < 0 || channel > 5) {
        std::cerr << "Error: Invalid channel value for temperature. Please provide a value between 0 and 5." << std::endl;
        // return a special value -100.0 to indicate an error
        return -999.0;
    }

    int ad_value = this->adc.read_channel(channel);
    float voltage = _AD_to_voltage(ad_value);
    float resistance = _voltage_to_temp_resistance(voltage);

    return resistance;
}

/// @brief Get the voltage of a given channel
/// @param channel The channel number of temperature sensor (0-7)
/// @return the voltage in volt
float Sensor::get_channel_voltage(int channel)
{
    // Ensure channel is within a valid range
    if (channel < 0 || channel > 7) {
        std::cerr << "Error: Invalid channel value for temperature. Please provide a value between 0 and 5." << std::endl;
        // return a special value -100.0 to indicate an error
        return -999.0;
    }

    int ad_value = this->adc.read_channel(channel);
    float voltage = _AD_to_voltage(ad_value);

    return voltage;
}