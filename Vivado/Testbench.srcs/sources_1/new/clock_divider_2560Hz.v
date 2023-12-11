`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/11/25 21:45:49
// Design Name: 
// Module Name: clock_divider_2560Hz
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


module clock_divider_2560Hz(
    input clk_in,           // Input clock, assumed to be 125 MHz
    output reg clk_out = 0  // Output for the divided frequency, starts at 0
);
    // Define a counter register, the size of which is based on the division ratio.
    // To divide 125 MHz down to 2560 Hz, the division ratio is 125000000 / 2560 = 48828.125.
    // Therefore, we need a counter that can count up to half of this value (since we toggle the output every half cycle).
    // Half of the division ratio is 24414.0625, so we will round up to 24415 for the counter threshold.
    // The minimum number of bits can be calculated by ceil(log2(24415)) which is 15 bits.
    reg [14:0] counter = 0; // 15-bit counter for the calculated division ratio

    // Calculate the required size of the counter
    // Since 125 MHz / 2560 Hz = 48828.125, we need to toggle the output every 24414.0625 cycles,
    // we'll use 24415 for simplicity.
    // Calculate the minimum required number of bits: ceil(log2(24415)) = 15

    always @(posedge clk_in) begin
        counter <= counter + 1; // Increment the counter by 1 on each rising edge of clk_in.
        // Check if the counter has reached the threshold to toggle the output clock.
        if (counter >= 24415) begin // When the time reaches (1 / (2 * 2560 Hz)) s.
            counter <= 0;            // Reset the counter to 0
            clk_out <= ~clk_out;    // Toggle the state of clk_out to generate the desired frequency
        end
    end

endmodule

