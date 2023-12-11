`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Muzhi Zhang
// 
// Create Date: 2023/11/13 16:30:19
// Design Name: 
// Module Name: clock_divider
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: change the clock frequency from 125 MHz to 256 Hz (255.95 Hz).
// 
// Dependencies: the input clock frequency should be 125 MHz
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module clock_divider(
    input clk_in,         // Input clock, now assumed to be 125 MHz
    output reg clk_out = 0 // Output for the divided frequency
);
    reg [17:0] counter = 0; // 18-bit counter for the counter because of the calculated division ratio

    // Calculate the required size of the counter
    // Since 125000000 Hz / 256 Hz ¡Ö 488281.25, we need a counter that can count at least up to 244140
    // (which is half of 488281.25) (toggle the logical level every 244140 clock ticks )
    // Calculate the minimum required number of bits: log2(244140) ¡Ö 18

    always @(posedge clk_in) begin
    // 
        counter <= counter + 1; // Increment the counter by 1 each time the clk_in signal has a rising edge.
        if (counter >= 244140) begin // Time reaches (1 / 512) s.
            counter <= 0;        // Reset the counter to 0
            clk_out <= ~clk_out; // Toggle the state of clk_out
        end
    end

endmodule