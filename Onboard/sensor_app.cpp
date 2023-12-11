#include "sensor.h"

int main() 
{
    Sensor sensor;

    float channel_temperature;

    for(int i = 0; ; i++)
    {
        channel_temperature = sensor.get_channel_temperature_lookup_table(0);
        printf("Channel temperature: %.2f\n", channel_temperature);
        delay(1000);
    }
    

    return 0;
}