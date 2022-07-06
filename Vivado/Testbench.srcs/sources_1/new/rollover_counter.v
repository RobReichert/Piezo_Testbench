`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 09.05.2022 08:59:57
// Design Name: 
// Module Name: rollover_counter
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


module rollover_counter #
    (
        parameter COUNTER_WIDTH = 32
    )
    
    (
        input  wire                                 aclk,
        input  wire                                 nRST,
        input  wire                                 en,
        input  wire [COUNTER_WIDTH-1:0]             MOD,
        output wire [COUNTER_WIDTH-1:0]             COUNT,
        output wire                                 THRESH
    );
    
    reg [COUNTER_WIDTH-1:0] count;
    reg [COUNTER_WIDTH-1:0] modulus;
    reg thresh;
    
    always @ (posedge aclk) begin
    
        modulus <= MOD;
        
        if (!nRST) begin
            count <= 0;
            thresh <= 0;
        end
    
        else if (count == (MOD - 1)) begin
            count <= 0;
            thresh <= 1;
        end
            
        else if (en) begin
            count <= count + 1;
            thresh <= 0;
        end    
    
    end
    
    assign THRESH = thresh;
    
endmodule
