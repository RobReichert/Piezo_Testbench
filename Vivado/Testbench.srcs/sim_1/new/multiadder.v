`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 06.08.2021 12:20:52
// Design Name: 
// Module Name: multiadder
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


module multiadder();

    reg clock;
    reg [31:0] MULT;
    reg [15:0] ADD;
    reg [13:0] ADC;
    wire [15:0] OUT;
    reg [13:0] sine [0:99];
    reg [7:0] i;
    
    AXI4_multi_adder ma (.S_AXIS_MULT_tdata(MULT),
                           .S_AXIS_MULT_tvalid(1'b1),
                           .S_AXIS_ADD_tdata(ADD),
                           .S_AXIS_ADD_tvalid(1'b1),
                           .S_AXIS_ADC_tdata({2'b0,ADC}),
                           .S_AXIS_ADC_tvalid(1'b1),
                           .M_AXIS_tdata(OUT),
                           .aclk(clock)
                            );
initial
begin
    clock = 1'b0;
    ADC = 0;
    MULT = 16'd1 << 16;
    ADD = 16'd1;
    i=0;
    
    sine[0] = 0;        sine[1] = 10;        sine[2] = 20;        sine[3] = 29;        sine[4] = 39;   
    sine[5] = 48;       sine[6] = 58;        sine[7] = 67;        sine[8] = 75;        sine[9] = 84;
    sine[10] = 92;      sine[11] = 100;      sine[12] = 107;      sine[13] = 114;      sine[14] = 120;
    sine[15] = 126;     sine[16] = 132;      sine[17] = 137;      sine[18] = 141;      sine[19] = 145;   
    sine[20] = 149;     sine[21] = 151;      sine[22] = 153;      sine[23] = 155;      sine[24] = 156;
    sine[25] = 156;     
end

always 
begin
    clock = 1'b1; 
    #4; // high for 4 * timescale = 4 ns

    clock = 1'b0; 
    #4; // low for 4 * timescale = 4 ns
end

always @(posedge clock)
begin
    // values for a and b
    if ( i < 25 )
       ADC = sine[i];
    else if ( i < 50 )
       ADC = sine[50 - i];
    else if  ( i < 75 )
       ADC = - sine[i - 50];
    else 
       ADC = - sine[100 - i];
    i = i+ 1;
    if(i == 100)
       i = 0;

    
end
endmodule