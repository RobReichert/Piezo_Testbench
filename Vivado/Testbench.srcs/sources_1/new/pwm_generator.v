`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/11/18 15:23:11
// Design Name: 
// Module Name: pwm_generator
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

module pwm_generator_safe(
    input clk,                       // Input clock with a frequency of approximately 256Hz
    input [7:0] DAC_value,           // 8-bit input value for PWM duty cycle
    output pwm_out                   // PWM output
);
    // defined by user:
    parameter enable_safety = 0;     // Parameter to enable(1) or disable(0) the safety feature
    parameter [4:0] safety_threshold = 16; // Threshold time (s) for the safety feature, 5-bit width, max. 31 seconds

    // Instantiate the original pwm_generator
    wire pwm_internal_out;
    pwm_generator pwm_gen_inst (
        .clk(clk),
        .DAC_value(DAC_value),
        .pwm_out(pwm_internal_out)
    );

    // Counter to track every 256 clock cycles
    reg [7:0] cycle_counter = 0;

    // Logic for safety feature
    reg [4:0] stability_counter = 0; // 5-bit counter for stability check
    reg [7:0] prev_DAC_value = 0;    // Register to store the previous value of DAC_value

    always @(posedge clk) begin
        // Decrement cycle counter and reset after 256 clock cycles (1 PWM period)
        if (cycle_counter == 0) begin
            cycle_counter <= 8'hFF;
            // Check if DAC_value has changed every 256 clock cycles (1 PWM period)
            if (prev_DAC_value != DAC_value) begin
                prev_DAC_value <= DAC_value;  // Update stored value
                stability_counter <= 0;       // Reset stability counter
            end else if (enable_safety && (stability_counter < safety_threshold)) begin
                stability_counter <= stability_counter + 1; // Increment stability counter
            end
        end else begin
            cycle_counter <= cycle_counter - 1;
        end
    end
/*    
    always @(posedge clk) begin
    // Increment cycle counter and reset after 256 cycles
        if (cycle_counter == 8'hFF) begin
            cycle_counter <= 0;
        end else begin
            cycle_counter <= cycle_counter + 1;
        end
    end
*/

    // Conditional assignment for safety mechanism
    assign pwm_out = (enable_safety && (stability_counter >= safety_threshold)) ? 1'b0 :
                     pwm_internal_out;

//assign pwm_out = pwm_internal_out;
endmodule


module pwm_generator(
    input clk,                 // Input clock with a frequency of approximately 256Hz
    input [7:0] DAC_value,     // 8-bit input value for comparison to determine PWM duty cycle
    output pwm_out     // PWM output, default to low
);

    // counter that can cover the full range of the 8-bit DAC_value
    reg [7:0] counter = 0;


    always @(posedge clk) begin
        // Increment the counter and reset if it exceeds the maximum 8-bit value
        if (counter >= 8'hFF) begin
            counter <= 0;
        end else begin
            counter <= counter + 1;
        end
    end
    
    // Continuous assignment with conditional operator for PWM output
    assign pwm_out = (counter < DAC_value) ? 1'b1 : 1'b0;

endmodule