`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Robert Reichert
// 
// Create Date: 04.07.2022 14:41:50
// Design Name: 
// Module Name: xadc_read
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
//
//Red Pitaya Pinout (Connector E3):
//AUX  0   1   8   9
//AI   A1  A2  A0  A3
//Pin  14  15  13  16



module xadc_read 
(
  // System signals
  input  wire       aclk,
  input  wire       aresetn,

  // Slave AXIS
  output wire        s_axis_tready,
  input  wire [15:0] s_axis_tdata,
  input  wire [4:0]  s_axis_tid,
  input  wire        s_axis_tvalid,
  
  // Master AXIS
  output wire          m_axis_tvalid,
  output wire [63:0]   m_axis_tdata
);

reg [15:0] dat_temp_reg, dat_aux0_reg, dat_aux1_reg, dat_aux8_reg, dat_aux9_reg;
reg [15:0] dat_temp_next, dat_aux0_next, dat_aux1_next, dat_aux8_next, dat_aux9_next;
reg valid_reg, valid_next;

always @(posedge aclk)
begin
    if(~aresetn)
    begin
      dat_temp_reg <= 16'b0;
      dat_aux0_reg <= 16'b0;
      dat_aux1_reg <= 16'b0;
      dat_aux8_reg <= 16'b0;
      dat_aux9_reg <= 16'b0;
      valid_reg <= 1'b0;
    end
    else
    begin
      dat_temp_reg <= dat_temp_next;
      dat_aux0_reg <= dat_aux0_next;
      dat_aux1_reg <= dat_aux1_next;
      dat_aux8_reg <= dat_aux8_next;
      dat_aux9_reg <= dat_aux9_next;
      valid_reg <= valid_next;
    end
end

always @*
begin
    dat_temp_next = dat_temp_reg;
    dat_aux0_next = dat_aux0_reg;
    dat_aux1_next = dat_aux1_reg;
    dat_aux8_next = dat_aux8_reg;
    dat_aux9_next = dat_aux9_reg;
    valid_next = valid_reg;
    case(s_axis_tid)
        5'd0: dat_temp_next = s_axis_tdata;
        5'd16: dat_aux0_next = s_axis_tdata;
        5'd17: dat_aux1_next = s_axis_tdata;
        5'd24: dat_aux8_next = s_axis_tdata;
        5'd25: dat_aux9_next = s_axis_tdata;
    endcase
end

assign m_axis_tdata = {dat_aux9_reg, dat_aux1_next, dat_aux0_next, dat_aux8_reg};
assign m_axis_tvalid = 1'b1;
assign s_axis_tready = 1'b1;

endmodule
