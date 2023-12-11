`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/11/20 15:30:39
// Design Name: 
// Module Name: testbench_pwm_generator
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


module testbench_pwm_generator;

    // Testbench signals
    // inputs
    reg clk_tb;                 // Testbench clock signal
    reg [7:0] DAC_value_tb;     // Testbench DAC_value signal
    // outputs
    wire pwm_out_tb;            // Testbench wire to observe PWM output

    // Instantiate the pwm_generator module (unit under test)
    pwm_generator uut (
        .clk(clk_tb),
        .DAC_value(DAC_value_tb),
        .pwm_out(pwm_out_tb)
    );

    // Generate the testbench clock with a period corresponding to 256Hz frequency
    initial begin
        clk_tb = 0; // Initialize the clock to 0
        // A clock period for 256Hz is approximately 3.90625 miliseconds
        // We use a 3.9ms period for simplicity
        forever #(3900000 / 2) clk_tb = ~clk_tb; // Toggle the clock every half period
    end

    // Test sequence
    initial begin
        // Initialize test conditions
        DAC_value_tb = 8'd0; // Start with a DAC value of 0
        #(3900000 * 256); // Wait for another 256 clock cycles
        
        // Test various DAC values and observe the PWM output behavior
        DAC_value_tb = 8'd64; // Set DAC_value to 25% duty cycle
        #(3900000 * 256*1.5); // Wait for another 256 clock cycles
        
        DAC_value_tb = 8'd128; // Set DAC_value to 50% duty cycle
        #(3900000 * 256*1.5); // Wait for 256 clock cycles to observe the PWM output       

        DAC_value_tb = 8'd192; // Set DAC_value to 75% duty cycle
        #(3900000 * 256); // Wait for another 256 clock cycles
        
        DAC_value_tb = 8'd255; // Set DAC_value to 100% duty cycle
        #(3900000 * 256); // Wait for another 256 clock cycles
        
        // Finish the test sequence
        $finish;
    end
/*
    // Optional VCD dump for waveform analysis
    initial begin
        $dumpfile("tb_pwm_generator.vcd");
        $dumpvars(0, tb_pwm_generator);
    end
    */
endmodule