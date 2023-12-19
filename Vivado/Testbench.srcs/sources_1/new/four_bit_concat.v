`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Muzhi Zhang
// 
// Create Date: 2023/11/26 11:06:01
// Design Name: 
// Module Name: four_bit_concat
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: combine the 4 input signals into a vector
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module four_bit_concat(
input dac_pwm_o_0,
input dac_pwm_o_1,
input dac_pwm_o_2,
input dac_pwm_o_3,
output [3:0] Dout
    );
    assign Dout = {dac_pwm_o_3, dac_pwm_o_2, dac_pwm_o_1, dac_pwm_o_0};
    
endmodule
