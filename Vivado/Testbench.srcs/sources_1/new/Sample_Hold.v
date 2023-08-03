`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Robert Reichert
// 
// Create Date: 11.08.2022 07:39:43
// Design Name: 
// Module Name: Sample_Hold
// Project Name: 
// Target Devices: 
// Tool Versions: 1.0
// Description: 
// 8 channel sampel and hold with integrated filter
// put filtered channel data to output every aclk/S_AXIS_CONFIG_tdata cycle
// exponential filter with tau = 1-(fil_a/2^CONFIG_WIDTH-1) / (faclk * (fil_a/2^CONFIG_WIDTH-1))
// fil_a calculation see line 138
//
// Dependencies: 
// 
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

module Sample_Hold # 
(
    parameter DATA_WIDTH = 16,
    parameter CONFIG_WIDTH = 16
)
(
    //inputs
    input  wire                                 aclk,
    input  wire                                 aresetn,
    input  wire [CONFIG_WIDTH-1:0]              S_AXIS_CONFIG_tdata,
    input  wire                                 S_AXIS_CONFIG_tvalid,
    input  wire [DATA_WIDTH-1:0]                S_AXIS_CANNEL_1_tdata,
    input  wire                                 S_AXIS_CANNEL_1_tvalid,
    input  wire [DATA_WIDTH-1:0]                S_AXIS_CANNEL_2_tdata,
    input  wire                                 S_AXIS_CANNEL_2_tvalid,
    input  wire [DATA_WIDTH-1:0]                S_AXIS_CANNEL_3_tdata,
    input  wire                                 S_AXIS_CANNEL_3_tvalid,
    input  wire [DATA_WIDTH-1:0]                S_AXIS_CANNEL_4_tdata,
    input  wire                                 S_AXIS_CANNEL_4_tvalid,
    input  wire [DATA_WIDTH-1:0]                S_AXIS_CANNEL_5_tdata,
    input  wire                                 S_AXIS_CANNEL_5_tvalid,
    input  wire [DATA_WIDTH-1:0]                S_AXIS_CANNEL_6_tdata,
    input  wire                                 S_AXIS_CANNEL_6_tvalid,
    input  wire [DATA_WIDTH-1:0]                S_AXIS_CANNEL_7_tdata,
    input  wire                                 S_AXIS_CANNEL_7_tvalid,
    input  wire [DATA_WIDTH-1:0]                S_AXIS_CANNEL_8_tdata,
    input  wire                                 S_AXIS_CANNEL_8_tvalid,
    //outputs
    output wire [DATA_WIDTH-1:0]                M_AXIS_CANNEL_1_tdata,
    output wire                                 M_AXIS_CANNEL_1_tvalid,
    output wire [DATA_WIDTH-1:0]                M_AXIS_CANNEL_2_tdata,
    output wire                                 M_AXIS_CANNEL_2_tvalid,
    output wire [DATA_WIDTH-1:0]                M_AXIS_CANNEL_3_tdata,
    output wire                                 M_AXIS_CANNEL_3_tvalid,
    output wire [DATA_WIDTH-1:0]                M_AXIS_CANNEL_4_tdata,
    output wire                                 M_AXIS_CANNEL_4_tvalid,
    output wire [DATA_WIDTH-1:0]                M_AXIS_CANNEL_5_tdata,
    output wire                                 M_AXIS_CANNEL_5_tvalid,
    output wire [DATA_WIDTH-1:0]                M_AXIS_CANNEL_6_tdata,
    output wire                                 M_AXIS_CANNEL_6_tvalid,
    output wire [DATA_WIDTH-1:0]                M_AXIS_CANNEL_7_tdata,
    output wire                                 M_AXIS_CANNEL_7_tvalid,
    output wire [DATA_WIDTH-1:0]                M_AXIS_CANNEL_8_tdata,
    output wire                                 M_AXIS_CANNEL_8_tvalid
);


////////////////////    
////Declarations////
////////////////////

    // sample register
    reg signed [DATA_WIDTH-1:0] sample_1, sample_2, sample_3, sample_4, sample_5, sample_6, sample_7, sample_8;
    reg signed [DATA_WIDTH-1:0] sample_1_next, sample_2_next, sample_3_next, sample_4_next, sample_5_next, sample_6_next, sample_7_next, sample_8_next;
    
    // valid register
    reg valid, valid_next;
    
    // filter registers
    reg signed [DATA_WIDTH-1:0] fil_1, fil_2, fil_3, fil_4, fil_5, fil_6, fil_7, fil_8;
    reg signed [DATA_WIDTH+CONFIG_WIDTH-1:0] fil_1_next, fil_2_next, fil_3_next, fil_4_next, fil_5_next, fil_6_next, fil_7_next, fil_8_next;
    reg [CONFIG_WIDTH-1:0] fil_a;
    
    //counter registers
    reg [CONFIG_WIDTH-1:0] count, count_next;
    
////////////////////    
////Math        ////
////////////////////   
always @(posedge aclk)
begin
    if(~aresetn)//initialisation
    begin
        fil_1 <= {(DATA_WIDTH){1'b0}};
        sample_1 <= S_AXIS_CANNEL_1_tdata;
        fil_2 <= {(DATA_WIDTH){1'b0}};
        sample_2 <= S_AXIS_CANNEL_2_tdata;
        fil_3 <= {(DATA_WIDTH){1'b0}};
        sample_3 <= S_AXIS_CANNEL_3_tdata;
        fil_4 <= {(DATA_WIDTH){1'b0}};
        sample_4 <= S_AXIS_CANNEL_4_tdata;
        fil_5 <= {(DATA_WIDTH){1'b0}};
        sample_5 <= S_AXIS_CANNEL_5_tdata;
        fil_6 <= {(DATA_WIDTH){1'b0}};
        sample_6 <= S_AXIS_CANNEL_6_tdata;
        fil_7 <= {(DATA_WIDTH){1'b0}};
        sample_7 <= S_AXIS_CANNEL_7_tdata;
        fil_8 <= {(DATA_WIDTH){1'b0}};
        sample_8 <= S_AXIS_CANNEL_8_tdata;
        valid <= 1'b0;
        count <= {(CONFIG_WIDTH){1'b0}};
        fil_a <= {(CONFIG_WIDTH){1'b1}};
    end
    else //move calculation to registers
    begin
        fil_1 <= fil_1_next >>> CONFIG_WIDTH;
        sample_1 <= sample_1_next;
        fil_2 <= fil_2_next >>> CONFIG_WIDTH;
        sample_2 <= sample_2_next;
        fil_3 <= fil_3_next >>> CONFIG_WIDTH;
        sample_3 <= sample_3_next;
        fil_4 <= fil_4_next >>> CONFIG_WIDTH;
        sample_4 <= sample_4_next;
        fil_5 <= fil_5_next >>> CONFIG_WIDTH;
        sample_5 <= sample_5_next;
        fil_6 <= fil_6_next >>> CONFIG_WIDTH;
        sample_6 <= sample_6_next;
        fil_7 <= fil_7_next >>> CONFIG_WIDTH;
        sample_7 <= sample_7_next;
        fil_8 <= fil_8_next >>> CONFIG_WIDTH;
        sample_8 <= sample_8_next;
        valid <= valid_next;
        count <= count_next;
        if (S_AXIS_CONFIG_tvalid && S_AXIS_CONFIG_tdata>8)
            fil_a <= {(CONFIG_WIDTH){1'b1}}/(S_AXIS_CONFIG_tdata>>3);
        else
            fil_a <= {(CONFIG_WIDTH){1'b1}};
    end
end
  
always @*
begin
    sample_1_next = sample_1;
    sample_2_next = sample_2;
    sample_3_next = sample_3;
    sample_4_next = sample_4;
    sample_5_next = sample_5;
    sample_6_next = sample_6;
    sample_7_next = sample_7;
    sample_8_next = sample_8;
    valid_next = 1'b0;
    count_next = count;
    
    //apply exponential filter
    if (fil_1[0:0] == 1 || fil_1[0:0] == 0) 
        fil_1_next = (({(CONFIG_WIDTH){1'b1}} - fil_a) * fil_1) + (fil_a * S_AXIS_CANNEL_1_tdata);
    else
        fil_1_next = {(DATA_WIDTH){1'b0}};
        
    if (fil_2[0:0] == 1 || fil_2[0:0] == 0) 
        fil_2_next = (({(CONFIG_WIDTH){1'b1}} - fil_a) * fil_2) + (fil_a * S_AXIS_CANNEL_2_tdata);
    else
        fil_2_next = {(DATA_WIDTH){1'b0}};
        
    if (fil_3[0:0] == 1 || fil_3[0:0] == 0) 
        fil_3_next = (({(CONFIG_WIDTH){1'b1}} - fil_a) * fil_3) + (fil_a * S_AXIS_CANNEL_3_tdata);
    else
        fil_3_next = {(DATA_WIDTH){1'b0}};
        
    if (fil_4[0:0] == 1 || fil_4[0:0] == 0) 
        fil_4_next = (({(CONFIG_WIDTH){1'b1}} - fil_a) * fil_4) + (fil_a * S_AXIS_CANNEL_4_tdata);
    else
        fil_4_next = {(DATA_WIDTH){1'b0}};
        
    if (fil_5[0:0] == 1 || fil_5[0:0] == 0) 
        fil_5_next = (({(CONFIG_WIDTH){1'b1}} - fil_a) * fil_5) + (fil_a * S_AXIS_CANNEL_5_tdata);
    else
        fil_5_next = {(DATA_WIDTH){1'b0}};
        
    if (fil_6[0:0] == 1 || fil_6[0:0] == 0) 
        fil_6_next = (({(CONFIG_WIDTH){1'b1}} - fil_a) * fil_6) + (fil_a * S_AXIS_CANNEL_6_tdata);
    else
        fil_6_next = {(DATA_WIDTH){1'b0}};
        
    if (fil_7[0:0] == 1 || fil_7[0:0] == 0) 
        fil_7_next = (({(CONFIG_WIDTH){1'b1}} - fil_a) * fil_7) + (fil_a * S_AXIS_CANNEL_7_tdata);
    else
        fil_7_next = {(DATA_WIDTH){1'b0}};
        
    if (fil_8[0:0] == 1 || fil_8[0:0] == 0) 
        fil_8_next = (({(CONFIG_WIDTH){1'b1}} - fil_a) * fil_8) + (fil_a * S_AXIS_CANNEL_8_tdata);
    else
        fil_8_next = {(DATA_WIDTH){1'b0}};
    
    //sample condition
    if (count >= S_AXIS_CONFIG_tdata-1) 
    begin
        sample_1_next = fil_1;
        sample_2_next = fil_2;
        sample_3_next = fil_3;
        sample_4_next = fil_4;
        sample_5_next = fil_5;
        sample_6_next = fil_6;
        sample_7_next = fil_7;
        sample_8_next = fil_8;
        count_next = {(CONFIG_WIDTH){1'b0}};
        valid_next = 1'b1;
    end
    else if (count[0:0] == 1 || count[0:0] == 0)
        count_next = count + 1'b1;
    else
        count_next = {(CONFIG_WIDTH){1'b0}};
end

////////////////////    
////Output      ////
////////////////////

assign M_AXIS_CANNEL_1_tdata = S_AXIS_CANNEL_1_tdata;
assign M_AXIS_CANNEL_1_tvalid = valid;
assign M_AXIS_CANNEL_2_tdata = S_AXIS_CANNEL_2_tdata;
assign M_AXIS_CANNEL_2_tvalid = valid;
assign M_AXIS_CANNEL_3_tdata = S_AXIS_CANNEL_3_tdata;
assign M_AXIS_CANNEL_3_tvalid = valid;
assign M_AXIS_CANNEL_4_tdata = S_AXIS_CANNEL_4_tdata;
assign M_AXIS_CANNEL_4_tvalid = valid;
assign M_AXIS_CANNEL_5_tdata = S_AXIS_CANNEL_5_tdata;
assign M_AXIS_CANNEL_5_tvalid = valid;
assign M_AXIS_CANNEL_6_tdata = S_AXIS_CANNEL_6_tdata;
assign M_AXIS_CANNEL_6_tvalid = valid;
assign M_AXIS_CANNEL_7_tdata = S_AXIS_CANNEL_7_tdata;
assign M_AXIS_CANNEL_7_tvalid = valid;
assign M_AXIS_CANNEL_8_tdata = S_AXIS_CANNEL_8_tdata;
assign M_AXIS_CANNEL_8_tvalid = valid;

endmodule
