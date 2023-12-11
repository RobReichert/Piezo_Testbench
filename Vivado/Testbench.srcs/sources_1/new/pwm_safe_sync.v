`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/11/25 22:22:28
// Design Name: 
// Module Name: pwm_safe_sync
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


module pwm_safe_sync(
    input clk,                  // Input clock with a frequency of approximately 2560Hz
    input [7:0] DAC_value,      // 8-bit input value for PWM duty cycle
    input sync,                 // Synchronization signal for the start of PWM period
    output reg pwm_out = 0      // PWM output, initialized to 0
);

    // Define a counter that will determine the PWM period (2560 cycles for 1 second)
    reg [11:0] counter = 0;     // 12-bit counter to count up to 2560
    reg sync_last = 0;          // Register to keep track of the last sync value
    
    // Process that generates the PWM signal
    always @(posedge clk) begin
        // Detect the rising edge of sync signal to start a new PWM cycle
        if (sync != sync_last) begin
            // Reset the counter when a change in sync signal is detected
            counter <= 0;
            pwm_out <= 1'b1;       // Start the PWM period with a high output
            if (DAC_value == 0) begin
                pwm_out <= 1'b0;
            end
            else begin
                pwm_out <= 1'b1;
            end
        end else if (counter >= 2560) begin
            // If one second has passed, pwm = 0
            //counter <= 0;
            pwm_out <= 1'b0;       // Start the PWM period with a low output
        end else begin
            // Increment the counter and generate the PWM signal based on the DAC_value
            counter <= counter + 1;
            if (counter < (DAC_value * 10)) begin
                pwm_out <= 1'b1;   // Keep the output high if within the duty cycle period
            end else begin
                pwm_out <= 1'b0;   // Set the output low once the duty cycle period is exceeded
            end
        end
        // Update the last sync value at the end of the clock cycle
        sync_last <= sync;
    end

endmodule

