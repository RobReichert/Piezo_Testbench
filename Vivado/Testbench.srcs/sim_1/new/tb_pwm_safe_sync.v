`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/11/25 22:46:49
// Design Name: 
// Module Name: tb_pwm_safe_sync
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


module tb_pwm_safe_sync;

    // Parameters of the pwm_safe_sync module
    parameter CLK_FREQ_HZ = 2560; // Frequency of the input clock
    parameter PERIOD_NS = 1000000000 / CLK_FREQ_HZ; // Period of the input clock in ns
    parameter PWM_PERIOD_SEC = 1; // PWM period in seconds
    parameter PWM_PERIOD_COUNT = PWM_PERIOD_SEC * CLK_FREQ_HZ; // PWM period in clock ticks

    // Testbench signals
    reg clk = 0;                // Test clock signal
    reg [7:0] DAC_value = 0;    // Test DAC value signal
    reg sync = 0;               // Test sync signal
    wire pwm_out;               // PWM output signal from the module

    // Instantiate the pwm_safe_sync module
    pwm_safe_sync uut (
        .clk(clk),
        .DAC_value(DAC_value),
        .sync(sync),
        .pwm_out(pwm_out)
    );

    // Generate the test clock
    always #(PERIOD_NS / 2) clk = ~clk;

    integer i;
    // Test sequence
    initial begin
        // Initialize signals
        DAC_value = 0;
        sync = 0;
        
        // Test 0% duty cycle
        sync = 1; // Trigger sync
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        
        // Test 25% duty cycle
        sync = 0; // Toggle sync
        DAC_value = 64; // Set DAC_value for 25% duty cycle
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        
        // Test 50% duty cycle
        sync = 1; // Toggle sync
        DAC_value = 128; // Set DAC_value for 50% duty cycle
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        
        // Test 75% duty cycle
        sync = 0; // Toggle sync
        DAC_value = 192; // Set DAC_value for 75% duty cycle
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        
        // Test 100% duty cycle
        sync = 1; // Toggle sync
        DAC_value = 255; // Set DAC_value for 100% duty cycle
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        
        // Test sync signal stopping for 5 seconds
        sync = 0; 
        // Keep sync low for 5 seconds (stop toggling)
        // Use a for loop to delay 1 second for 5 iterations
        for (i = 0; i < 5; i = i + 1) begin
            #(PWM_PERIOD_COUNT * PERIOD_NS); // Delay for 1 second
        end
        
        // Test resuming sync toggling
        sync = 1; // Resume sync toggling
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        sync = 0;
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        
        sync = 1; // Toggle sync
        DAC_value = 192; // Set DAC_value for 75% duty cycle
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        
        sync = 0; // Toggle sync
        DAC_value = 128; // Set DAC_value for 50% duty cycle
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        
        sync = 1; // Toggle sync
        DAC_value = 64; // Set DAC_value for 25% duty cycle
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        

        sync = 0; // Trigger sync
        DAC_value = 0; // Set DAC_value for 0% duty cycle
        #(PWM_PERIOD_COUNT * PERIOD_NS);
        
        $finish; // End simulation
    end

endmodule
