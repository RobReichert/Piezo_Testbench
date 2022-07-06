`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 05.08.2021 15:32:46
// Design Name: 
// Module Name: testbench
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module testbench();

    reg clock_p;
    reg clock_n;
    reg [15:0] adc_in;
    wire [13:0] dac_out;
    reg signed [13:0] sine [0:99];
    reg [7:0] i;
    
    system_wrapper sw (.adc_clk_p_i(clock_p),
                       .adc_clk_n_i(clock_n),
                       .adc_dat_a_i(adc_in),
                       .dac_dat_o(dac_out)
                       );
initial
begin
    clock_p = 1'b0;
    clock_n = 1'b1;
    adc_in = 0;
    i=0;
    
    sine[0] = 0;        sine[1] = 10;        sine[2] = 20;        sine[3] = 29;        sine[4] = 39;   
    sine[5] = 48;       sine[6] = 58;        sine[7] = 67;        sine[8] = 75;        sine[9] = 84;
    sine[10] = 92;      sine[11] = 100;      sine[12] = 107;      sine[13] = 114;      sine[14] = 120;
    sine[15] = 126;     sine[16] = 132;      sine[17] = 137;      sine[18] = 141;      sine[19] = 145;   
    sine[20] = 149;     sine[21] = 151;      sine[22] = 153;      sine[23] = 155;      sine[24] = 156;
    sine[25] = 156;     
end

always 
begin
    clock_p = 1'b1; 
    clock_n = 1'b0;
    #4; // high for 4 * timescale = 4 ns

    clock_p = 1'b0; 
    clock_n = 1'b1;
    #4; // low for 4 * timescale = 4 ns
end

always @(posedge clock_p)
begin
    // values for a and b
    if ( i < 25 )
       adc_in = sine[i];
    else if ( i < 50 )
       adc_in = sine[50 - i];
    else if  ( i < 75 )
       adc_in = - sine[i - 50];
    else 
       adc_in = - sine[100 - i];
    i = i+ 1;
    if(i == 100)
       i = 0;

    
end
endmodule
