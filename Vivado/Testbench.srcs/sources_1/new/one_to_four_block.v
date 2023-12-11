`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/11/13 16:35:36
// Design Name: 
// Module Name: one_to_four_block
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


module one_to_four_block(
input dac_pwm_o_0,
input dac_pwm_o_1,
input dac_pwm_o_2,
input dac_pwm_o_3,
output [3:0] Dout
    );
    assign Dout = {1'b1,1'b0,1'b0,dac_pwm_o_0};
    
endmodule