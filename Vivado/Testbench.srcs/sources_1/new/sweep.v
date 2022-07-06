`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Chris Cameron
// 
// Create Date: 10.2021 
// Design Name: 
// Module Name: freq_sweep
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


module freq_sweep # 
(
    parameter PHASE_WIDTH = 30,
    parameter FREQ_WIDTH = 16,
    parameter DURATION_WIDTH = 8,
    parameter AXIS_TDATA_WIDTH = 32,
    parameter SELECT_WIDTH = 2
)
(
    input  wire                                 aclk,
    input  wire                                 trig,
    input [FREQ_WIDTH - 1:0]                    start_freq,
    input [FREQ_WIDTH - 1:0]                    stop_freq,
    input [DURATION_WIDTH - 1:0]                duration,
    input [SELECT_WIDTH - 1:0]                  sel,
    
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output reg [AXIS_TDATA_WIDTH-1:0]           M_AXIS_phase_tdata,
    output wire                                 M_AXIS_phase_tvalid,
    output reg                                  trig_out

);
    localparam FCLK = 125000000;
    
    reg [PHASE_WIDTH - 1:0] phase;
    reg [PHASE_WIDTH - 1:0] start_phase;
    reg [PHASE_WIDTH - 1:0] stop_phase;
    reg [PHASE_WIDTH - 1:0] phase_span;
    reg [PHASE_WIDTH - 1:0] interval = 0;
    reg sweep_active;
    reg [PHASE_WIDTH - 1:0] counter = 0;
    
    always @ (start_freq, stop_freq, duration)
    begin
        start_phase = (start_freq << PHASE_WIDTH) / FCLK; // x << 30 / 125000000 ~= 8x
        stop_phase = (stop_freq << PHASE_WIDTH) / FCLK;    
        phase_span = stop_phase - start_phase;
        // Approach the stepping by incrementing by 1 every x cycles - setting a fractional increment each cycle gets a bit dicey at slow small sweeps
        interval = (duration * 125000000 / phase_span);
    end
    
    always @ (posedge aclk)
    begin
        if (trig)
        begin
            sweep_active = 1;
            trig_out = 1;
        end
        
        if (sweep_active)
        begin    
            counter = counter + 1;          
            
            if (counter > interval) 
            begin
                counter = 0;
                phase = phase + 1;
                if (phase >= stop_phase)
                    sweep_active = 0;
            end
            
        end
        
        else 
            sweep_active = 0;
                
    end        
     
    always @* 
    begin
        M_AXIS_phase_tdata <= phase; 
        
        case (sel)  
            2'b00 : M_AXIS_phase_tdata = phase;       
            2'b01 : M_AXIS_phase_tdata = {16'b0};      
            2'b10 : M_AXIS_phase_tdata = {16'b1};      
            2'b11 : M_AXIS_phase_tdata = {8'b0, 8'b1};      
        endcase
    end                    
    
    assign M_AXIS_phase_tvalid = 1;
  
    
endmodule