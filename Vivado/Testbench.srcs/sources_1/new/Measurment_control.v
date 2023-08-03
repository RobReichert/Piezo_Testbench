`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Robert Reichert
// 
// Create Date: 02.08.2023 10:16:07
// Design Name: 
// Module Name: Measurment_control
// Project Name: 
// Target Devices: 
// Tool Versions: 0.1
// Description: 
// Trigger measurement after DDS cos is at minimum
// if DDS [31:16] is at minimum value and measure is high --> is_measure will get high
// is_measure is reseted by measure = low
//
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module Measurment_control
(
    //inputs
    input  wire                                 aclk,
    input  wire                                 aresetn,
    input  wire [31:0]                          S_AXIS_dds_in_tdata,
    input  wire                                 S_AXIS_dds_in_tvalid,
    input  wire                                 measure,
    //outputs
    output wire [31:0]                          M_AXIS_dds_out_tdata,
    output wire                                 M_AXIS_dds_out_tvalid,
    output wire                                 is_measure
);

reg set, set_next;

always @(posedge aclk)
begin
    if(~aresetn)//initialisation
    begin
        set <= 1'b0;
    end
    else //move calculation to registers
    begin
        set <= set_next;
    end
end

always @*
begin
    set_next = set;
    
    //RS Flipflop
    if (measure == 0) 
        set_next = 1'b0;
    else if (S_AXIS_dds_in_tdata[31:18]==14'b10000000000000) //lower bytes are scipped to get a biger range for trigger signal
        set_next = 1'b1;
end

assign is_measure = set;
assign M_AXIS_dds_out_tdata = S_AXIS_dds_in_tdata;
assign M_AXIS_dds_out_tvalid = S_AXIS_dds_in_tvalid;

endmodule
