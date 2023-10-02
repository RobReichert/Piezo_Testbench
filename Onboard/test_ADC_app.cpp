#include "adc128d818.h" 

int main(void)
{
    ADC128D818 adc;

    // read channel value
    
    int16_t val_channel_0;
    int16_t val_channel_1;
    int16_t val_channel_2;
    int16_t val_channel_3;
    int16_t val_channel_4;
    int16_t val_channel_5;
    int16_t val_channel_6;
    int16_t val_channel_7;

    for(int i=1;;i++)
    {
        val_channel_0 = adc.read_channel(0);
        printf("(12 bit) channel value 0: %d\n\n", val_channel_0);
        

        val_channel_1 = adc.read_channel(1);
        printf("(12 bit) channel value 1: %d\n\n", val_channel_1);


        val_channel_2 = adc.read_channel(2);
        printf("(12 bit) channel value 2: %d\n\n", val_channel_2);


        val_channel_3 = adc.read_channel(3);
        printf("(12 bit) channel value 3: %d\n\n", val_channel_3);


        val_channel_4 = adc.read_channel(4);
        printf("(12 bit) channel value 4: %d\n\n", val_channel_4);
        

        val_channel_5 = adc.read_channel(5);
        printf("(12 bit) channel value 5: %d\n\n", val_channel_5);


        val_channel_6 = adc.read_channel(6);
        printf("(12 bit) channel value 6: %d\n\n", val_channel_6);


        val_channel_7 = adc.read_channel(7);
        printf("(12 bit) channel value 7: %d\n\n", val_channel_7);

        delay(1000);
    }

}