// (c) Copyright 1995-2022 Xilinx, Inc. All rights reserved.
// 
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
// 
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
// 
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
// 
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
// 
// DO NOT MODIFY THIS FILE.


// IP VLNV: xilinx.com:module_ref:feedback_combined:1.0
// IP Revision: 1

(* X_CORE_INFO = "feedback_combined,Vivado 2020.2" *)
(* CHECK_LICENSE_TYPE = "system_feedback_combined_0_0,feedback_combined,{}" *)
(* CORE_GENERATION_INFO = "system_feedback_combined_0_0,feedback_combined,{x_ipProduct=Vivado 2020.2,x_ipVendor=xilinx.com,x_ipLibrary=module_ref,x_ipName=feedback_combined,x_ipVersion=1.0,x_ipCoreRevision=1,x_ipLanguage=VERILOG,x_ipSimLanguage=MIXED,ADC_DATA_WIDTH=16,DDS_OUT_WIDTH=16,PARAM_WIDTH=32,PARAM_A_OFFSET=0,PARAM_B_OFFSET=32,PARAM_C_OFFSET=64,PARAM_D_OFFSET=96,PARAM_E_OFFSET=128,PARAM_F_OFFSET=160,PARAM_G_OFFSET=192,PARAM_H_OFFSET=224,DAC_DATA_WIDTH=14,CFG_WIDTH=288,AXIS_TDATA_WIDTH=16,SELECT_WIDTH=2,CONTINUOUS_O\
UTPUT=1}" *)
(* IP_DEFINITION_SOURCE = "module_ref" *)
(* DowngradeIPIdentifiedWarnings = "yes" *)
module system_feedback_combined_0_0 (
  aclk,
  trig_in,
  sel,
  S_AXIS_CFG_tdata,
  S_AXIS_CFG_tvalid,
  S_AXIS_ADC1_tdata,
  S_AXIS_ADC1_tvalid,
  S_AXIS_ADC2_tdata,
  S_AXIS_ADC2_tvalid,
  M_AXIS_tdata,
  M_AXIS_tvalid,
  trig_out
);

(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME aclk, ASSOCIATED_BUSIF M_AXIS:S_AXIS_ADC1:S_AXIS_ADC2:S_AXIS_CFG, FREQ_HZ 125000000, FREQ_TOLERANCE_HZ 0, PHASE 0.0, CLK_DOMAIN system_pll_0_0_clk_out1, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:signal:clock:1.0 aclk CLK" *)
input wire aclk;
input wire trig_in;
input wire [1 : 0] sel;
(* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 S_AXIS_CFG TDATA" *)
input wire [287 : 0] S_AXIS_CFG_tdata;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME S_AXIS_CFG, FREQ_HZ 125000000, TDATA_NUM_BYTES 36, TDEST_WIDTH 0, TID_WIDTH 0, TUSER_WIDTH 0, HAS_TREADY 0, HAS_TSTRB 0, HAS_TKEEP 0, HAS_TLAST 0, PHASE 0.0, CLK_DOMAIN system_pll_0_0_clk_out1, LAYERED_METADATA undef, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 S_AXIS_CFG TVALID" *)
input wire S_AXIS_CFG_tvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 S_AXIS_ADC1 TDATA" *)
input wire [15 : 0] S_AXIS_ADC1_tdata;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME S_AXIS_ADC1, FREQ_HZ 125000000, TDATA_NUM_BYTES 2, TDEST_WIDTH 0, TID_WIDTH 0, TUSER_WIDTH 0, HAS_TREADY 0, HAS_TSTRB 0, HAS_TKEEP 0, HAS_TLAST 0, PHASE 0.0, CLK_DOMAIN system_pll_0_0_clk_out1, LAYERED_METADATA undef, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 S_AXIS_ADC1 TVALID" *)
input wire S_AXIS_ADC1_tvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 S_AXIS_ADC2 TDATA" *)
input wire [15 : 0] S_AXIS_ADC2_tdata;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME S_AXIS_ADC2, FREQ_HZ 125000000, TDATA_NUM_BYTES 2, TDEST_WIDTH 0, TID_WIDTH 0, TUSER_WIDTH 0, HAS_TREADY 0, HAS_TSTRB 0, HAS_TKEEP 0, HAS_TLAST 0, PHASE 0.0, CLK_DOMAIN system_pll_0_0_clk_out1, LAYERED_METADATA undef, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 S_AXIS_ADC2 TVALID" *)
input wire S_AXIS_ADC2_tvalid;
(* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 M_AXIS TDATA" *)
output wire [15 : 0] M_AXIS_tdata;
(* X_INTERFACE_PARAMETER = "XIL_INTERFACENAME M_AXIS, FREQ_HZ 125000000, TDATA_NUM_BYTES 2, TDEST_WIDTH 0, TID_WIDTH 0, TUSER_WIDTH 0, HAS_TREADY 0, HAS_TSTRB 0, HAS_TKEEP 0, HAS_TLAST 0, PHASE 0.0, CLK_DOMAIN system_pll_0_0_clk_out1, LAYERED_METADATA undef, INSERT_VIP 0" *)
(* X_INTERFACE_INFO = "xilinx.com:interface:axis:1.0 M_AXIS TVALID" *)
output wire M_AXIS_tvalid;
output wire trig_out;

  feedback_combined #(
    .ADC_DATA_WIDTH(16),
    .DDS_OUT_WIDTH(16),
    .PARAM_WIDTH(32),
    .PARAM_A_OFFSET(0),
    .PARAM_B_OFFSET(32),
    .PARAM_C_OFFSET(64),
    .PARAM_D_OFFSET(96),
    .PARAM_E_OFFSET(128),
    .PARAM_F_OFFSET(160),
    .PARAM_G_OFFSET(192),
    .PARAM_H_OFFSET(224),
    .DAC_DATA_WIDTH(14),
    .CFG_WIDTH(288),
    .AXIS_TDATA_WIDTH(16),
    .SELECT_WIDTH(2),
    .CONTINUOUS_OUTPUT(1)
  ) inst (
    .aclk(aclk),
    .trig_in(trig_in),
    .sel(sel),
    .S_AXIS_CFG_tdata(S_AXIS_CFG_tdata),
    .S_AXIS_CFG_tvalid(S_AXIS_CFG_tvalid),
    .S_AXIS_ADC1_tdata(S_AXIS_ADC1_tdata),
    .S_AXIS_ADC1_tvalid(S_AXIS_ADC1_tvalid),
    .S_AXIS_ADC2_tdata(S_AXIS_ADC2_tdata),
    .S_AXIS_ADC2_tvalid(S_AXIS_ADC2_tvalid),
    .M_AXIS_tdata(M_AXIS_tdata),
    .M_AXIS_tvalid(M_AXIS_tvalid),
    .trig_out(trig_out)
  );
endmodule
