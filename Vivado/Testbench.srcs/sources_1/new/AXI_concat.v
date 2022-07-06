`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 17.06.2021 11:46:27
// Design Name: 
// Module Name: AXI_mux
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


module AXI_concat #
(
    parameter AXIS_TDATA_WIDTH = 16,
    parameter INPUT_CHANNELS = 2
)
(
    input aclk,
      
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input [AXIS_TDATA_WIDTH-1:0]        S_AXIS_PORT1_tdata,
    input                               S_AXIS_PORT1_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input [AXIS_TDATA_WIDTH-1:0]        S_AXIS_PORT2_tdata,
    input                               S_AXIS_PORT2_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output [AXIS_TDATA_WIDTH*INPUT_CHANNELS-1:0]    M_AXIS_tdata,
    output                                          M_AXIS_tvalid 
);
   
    assign M_AXIS_tdata = {S_AXIS_PORT1_tdata, S_AXIS_PORT2_tdata};

endmodule
   