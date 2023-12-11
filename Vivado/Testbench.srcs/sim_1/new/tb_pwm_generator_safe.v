`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/11/20 16:37:42
// Design Name: 
// Module Name: tb_pwm_generator_safe
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


module tb_pwm_generator_safe;

    // Testbench clock signal with a frequency of approximately 256Hz
    reg clk_tb;
    // Testbench DAC_value signal
    reg [7:0] DAC_value_tb;
    // Testbench wire to observe the PWM output
    wire pwm_out_tb;

    // Instantiate the pwm_generator_safe module with testbench signals
    pwm_generator_safe #(
        .enable_safety(1),      // Enable the safety feature
        .safety_threshold(16)   // Set the safety threshold
    ) uut (
        .clk(clk_tb),
        .DAC_value(DAC_value_tb),
        .pwm_out(pwm_out_tb)
    );

    // Generate the testbench clock with a period corresponding to 256Hz frequency
    initial begin
        clk_tb = 0; // Initialize the clock to 0
        // Clock period for 256Hz is approximately 3.9ms (3900000ns)
        forever #(3900000 / 2) clk_tb = ~clk_tb; // Toggle the clock every half period
    end

    integer i;
    // Test sequence process
    initial begin
        // Initialize DAC_value to 0
        DAC_value_tb = 8'd0;
        #(3900000 * 256 * 1); // Observe for 1*256 clock cycles
        
        // Change DAC_value again to resume normal operation
        DAC_value_tb = 8'd64; // Change DAC_value to see if pwm duty cycle is 25%
        #(3900000 * 256 * 1.5); // Observe for 2*256 clock cycles
        
        /*
        // Wait for a few clock cycles
        #(3900000 * 10); // Wait for 10 clock cycles at 256Hz
        */

        // Apply different DAC_values and observe the PWM behavior
        DAC_value_tb = 8'd128; // 50% duty cycle
        #(3900000 * 256 * 1.5); // Observe for 2*256 clock cycles
        
        DAC_value_tb = 8'd192; // 50% duty cycle
        #(3900000 * 256 * 2); // Observe for 2*256 clock cycles
        
        // Test the safety feature by not changing DAC_value
        // Expect pwm_out to go low after the specified safety threshold
        DAC_value_tb = 8'd255; // Set a constant DAC_value
        
        
        for(i=0;i<33;i=i+1) begin // Wait for 20*256 clock cycles to trigger the safety feature
            #(3900000 * 256 * 1);
        end
        
        // #(3900000 * 256 * 20); // cannot be executed because 3900000 * 256 * 20 > 2^31-1
        
        // Change DAC_value again to resume normal operation
        DAC_value_tb = 8'd64; // Change DAC_value to see if pwm_out resumes
        #(3900000 * 256 * 2); // Observe for 2*256 clock cycles

        // End the simulation
        $finish;
    end
    /*
    // Optional VCD dump for waveform analysis
    initial begin
        $dumpfile("tb_pwm_generator_safe.vcd");
        $dumpvars(0, tb_pwm_generator_safe);
    end
    */

endmodule