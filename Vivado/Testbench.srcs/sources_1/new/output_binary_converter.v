`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 12.08.2021 16:35:28
// Design Name: 
// Module Name: output_binary_converter
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


module output_binary_converter #
(
    parameter TDATA_WIDTH = 16,
    parameter integer DAC_DATA_WIDTH = 14
)
(
    input  wire                         aclk,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
input [TDATA_WIDTH-1:0]                 S_AXIS_tdata,
    input                               S_AXIS_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output wire [TDATA_WIDTH-1:0]       M_AXIS_tdata,
    output wire                         M_AXIS_tvalid

);
localparam PADDING_WIDTH = TDATA_WIDTH - DAC_DATA_WIDTH;
    
//assign M_AXIS_tdata = {{(PADDING_WIDTH + 1){S_AXIS_tdata[DAC_DATA_WIDTH-1]}}, ~S_AXIS_tdata[DAC_DATA_WIDTH-2:0]};
assign M_AXIS_tdata = {S_AXIS_tdata}; // Just pass through as test - test with feedback before removing this module completely.
assign M_AXIS_tvalid = S_AXIS_tvalid;
    
endmodule
