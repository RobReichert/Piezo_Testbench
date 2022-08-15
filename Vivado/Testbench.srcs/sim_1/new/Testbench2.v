`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 11.08.2022 10:03:50
// Design Name: 
// Module Name: Testbench2
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


module Testbench2;
//Parameters
parameter NR_CHANNEL = 8;
parameter DATA_WIDTH = 16;
parameter CONFIG_WIDTH = 16;

//inputs
reg aclk;
reg aresetn;
reg [CONFIG_WIDTH-1:0]              S_AXIS_CONFIG_tdata;
reg                                 S_AXIS_CONFIG_tvalid;
reg signed [DATA_WIDTH-1:0]                S_AXIS_CANNEL_1_tdata;
reg                                 S_AXIS_CANNEL_1_tvalid;
//outputs
wire [DATA_WIDTH-1:0]               M_AXIS_CANNEL_1_tdata;
wire                                M_AXIS_CANNEL_1_tvalid;

//Sinus definition
reg [13:0] sine [0:25];

//Define unit under Test
Sample_Hold uut(
    .aclk (aclk),
    .aresetn(aresetn),
    .S_AXIS_CONFIG_tdata(S_AXIS_CONFIG_tdata),
    .S_AXIS_CONFIG_tvalid(S_AXIS_CONFIG_tvalid),
    .S_AXIS_CANNEL_7_tdata(S_AXIS_CANNEL_1_tdata),
    .S_AXIS_CANNEL_7_tvalid(S_AXIS_CANNEL_1_tvalid),
    .M_AXIS_CANNEL_7_tdata(M_AXIS_CANNEL_1_tdata),
    .M_AXIS_CANNEL_7_tvalid(M_AXIS_CANNEL_1_tvalid)
);

integer k = 0;
integer i = 0;

initial
begin    
    //define sine
    sine[0] = 0;        sine[1] = 10;        sine[2] = 20;        sine[3] = 29;        sine[4] = 39;   
    sine[5] = 48;       sine[6] = 58;        sine[7] = 67;        sine[8] = 75;        sine[9] = 84;
    sine[10] = 92;      sine[11] = 100;      sine[12] = 107;      sine[13] = 114;      sine[14] = 120;
    sine[15] = 126;     sine[16] = 132;      sine[17] = 137;      sine[18] = 141;      sine[19] = 145;   
    sine[20] = 149;     sine[21] = 151;      sine[22] = 153;      sine[23] = 155;      sine[24] = 156;
    sine[25] = 156;
    
    //Setup
    S_AXIS_CONFIG_tdata=100;
    S_AXIS_CANNEL_1_tdata=500;
    S_AXIS_CANNEL_1_tvalid=1;
    S_AXIS_CONFIG_tvalid=1;

    
    aresetn=0;
    aclk=0;
    #5
    aresetn=1;
    #5
    for (k=0; k<2000; k=k+1)
    begin
        i=k/20;
        if ( i < 25 )
            S_AXIS_CANNEL_1_tdata = sine[i]*200;
        else if ( i < 50 )
            S_AXIS_CANNEL_1_tdata = sine[50 - i]*200;
        else if  ( i < 75 )
            S_AXIS_CANNEL_1_tdata = - sine[i - 50]*200;
        else 
            S_AXIS_CANNEL_1_tdata = - sine[100 - i]*200;
        #10
        aclk=1;
        #10
        aclk=0;
    end
    #5 $finish;
end
endmodule
