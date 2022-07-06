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


module xadc_read 
(
  // System signals
  input  wire       aclk,
  input  wire       aresetn,
  input  wire [4:0] channel,
  input  wire       eoc,
  
  // Master DRP
  output wire         m_drp_den,
  output wire         m_drp_dwe,
  output wire [6:0]   m_drp_daddr,
  output wire [15:0]  m_drp_di,
  input  wire         m_drp_drdy, 
  input  wire [15:0]  m_drp_do,

  // Master AXIS
  output wire          m_axis_tvalid,
  output wire [31:0]   m_axis_tdata
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
    case(channel)
        5'd0: dat_temp_next = 16'b0;
        5'd16: dat_aux0_next = 16'b0;
        5'd17: dat_aux1_next = 16'b0;
        5'd24: dat_aux8_next = 16'b0;
        5'd25: dat_aux9_next = 16'b0;
    endcase
end

assign m_axis_tdata = {15'b0,m_drp_drdy,11'b0, channel};
assign m_axis_tvalid = 1'b1;

assign m_drp_dwe = 1'b0;
assign m_drp_di = 16'b0;
assign m_drp_daddr = {2'b0, channel};
assign m_drp_den = eoc;

endmodule
