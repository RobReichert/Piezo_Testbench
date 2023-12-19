`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Muzhi Zhang
// 
// Create Date: 2023/11/25 21:49:34
// Design Name: 
// Module Name: tb_clock_divider_2560Hz
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: to simulate the behaviour of module clock_divider_2560Hz
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module tb_clock_divider_2560Hz;

    // Define the input clock frequency and period for simulation
    parameter CLK_IN_FREQ = 125e6; // Input clock frequency of 125MHz
    parameter CLK_IN_PERIOD = 1e9 / CLK_IN_FREQ; // Input clock period

    // Declare signals for the testbench
    reg clk_in; // Input clock signal
    wire clk_out; // Output clock signal

    // Instantiate the Unit Under Test (UUT)
    clock_divider_2560Hz uut (
        .clk_in(clk_in),
        .clk_out(clk_out)
    );

    // Generate the input clock signal with a period defined by CLK_IN_PERIOD
    initial begin
        clk_in = 0; // Initialize the clock to 0
        forever #(CLK_IN_PERIOD / 2) clk_in = ~clk_in; // Toggle the clock every half period
    end

    // Main testbench control logic
    initial begin
        // Initialize the simulation environment
        //$dumpfile("tb_clock_divider.vcd"); // Define the waveform output file
        //$dumpvars(0, tb_clock_divider); // Save all waveform data

        // Stop the simulation after a certain period of time
        #100000000; // Run for a certain time, e.g., 100ms
        $finish; // End the simulation
    end

endmodule
