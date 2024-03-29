# FPGA
The shered memory consisting of two files (mem and cma) both located in the dev folder.
To get it working like this the SD card image of Parvel Demin is nessecery, it can be installed to the RedPitaya according the description in the LED blinker tutorial:
http://pavel-demin.github.io/red-pitaya-notes/led-blinker/

## sts
Located in mem file with memory offset of 0x40000000 and range of 4k --> FPGA output

Bit      | Offset | Signal               | Substructure
-------- | ------ | -------------------- | ------
[15:0]   | 0      | ram writer position counter | 
[47:32]  | 4      | system feedback      | [32:32] = measurement is running


## cfg
Located in mem file with memory offset of 0x40001000 and range of 4k --> FPGA input

Bit      | Byte Offset | Signal                          | Substructure
-------- | ----------- | ------------------------------- | ------
[7:0]    | 0           | system input commands           | [0:0]=measure
&nbsp;   |             |                                 | [1:2]=Lock in amp mode 
[15:8]   | 1           | Measurement modes               | [11:8]=Load mode
&nbsp;   |             |                                 | [15:12]=Sample mode
[31:16]  | 2           | sample rate (Sample/Hold Config --> f=125MHz/value) |
[63:32]  | 4           | RAM_adress --> set address of writer | 
[95:64]  | 8           | DDS phase (phase=f/125MHz*2^30-1) |
[127:96] | 12          | PWM DAC Value                   | [103:96] DAC 1 (Offset 12)
&nbsp;   |             |                                 | [111:104] DAC 2 (Offset 13)
&nbsp;   |             |                                 | [119:112] DAC 3 (Offset 14)
&nbsp;   |             |                                 | [127:120] DAC 4 (Offset 15)
[383:128]| 16          | Test Parameters                 | [159:128] L1 (Offset 16)
&nbsp;   |             |                                 | [191:160] L2 (Offset 20)
&nbsp;   |             |                                 | [223:192] L3 (Offset 24)
&nbsp;   |             |                                 | [255:234] L4 (Offset 28)
&nbsp;   |             |                                 | [287:256] S1 (Offset 32)
&nbsp;   |             |                                 | [319:288] S1 (Offset 36)
&nbsp;   |             |                                 | [351:320] S1 (Offset 40)
&nbsp;   |             |                                 | [383:352] S1 (Offset 44)
[431:384]| 48          | Controller Parameters           | [399:384] P (Offset 48)
&nbsp;   |             |                                 | [415:400] I (Offset 50)
&nbsp;   |             |                                 | [431:416] D (Offset 52)

## ram
Located in cma file with memory offset of 0x00000000 and range of 512M

8 channel with 16bit each arranched in 128bit blocks

e.g. [0:15]ch1,[16:31]ch2,[32:47]ch3,[48:63]ch4,[64:79]ch5,[80:95]ch6,[96:111]ch7,[112:127]ch8,[128:143]ch1,...

Data of each channel have to be defined in data_aquisiton section of vivado blockdesign