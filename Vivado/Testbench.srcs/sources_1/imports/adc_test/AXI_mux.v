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


module AXI_mux #
(
    parameter AXIS_TDATA_WIDTH = 32,
    parameter INPUT_CHANNELS = 3
)
(
    input [1:0] select,
    input aclk,
      
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input [AXIS_TDATA_WIDTH-1:0]        S_AXIS_PORT1_tdata,
    input                               S_AXIS_PORT1_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input [AXIS_TDATA_WIDTH-1:0]        S_AXIS_PORT2_tdata,
    input                               S_AXIS_PORT2_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input [AXIS_TDATA_WIDTH-1:0]        S_AXIS_PORT3_tdata,
    input                               S_AXIS_PORT3_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output reg [AXIS_TDATA_WIDTH-1:0]  M_AXIS_tdata,
    output reg                         M_AXIS_tvalid 
);
   
    always@*
    
    case(select)
        2'b01 : begin
                    assign M_AXIS_tdata = S_AXIS_PORT1_tdata;
                    assign M_AXIS_tvalid = S_AXIS_PORT1_tvalid;
                end
                
        2'b10 : begin
                    assign M_AXIS_tdata = S_AXIS_PORT2_tdata;
                    assign M_AXIS_tvalid = S_AXIS_PORT2_tvalid;
                end
                
        2'b11 : begin
                    assign M_AXIS_tdata = S_AXIS_PORT3_tdata;
                    assign M_AXIS_tvalid = S_AXIS_PORT3_tvalid;
                end     
                        
        default : begin
                    assign M_AXIS_tdata = 32'b0;
                    assign M_AXIS_tvalid = 0;
                end            
    endcase

endmodule
   