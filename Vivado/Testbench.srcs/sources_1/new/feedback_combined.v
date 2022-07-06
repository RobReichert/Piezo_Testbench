`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: Chris Cameron
// 
// Create Date: 04.11.2021 13:50:12
// Design Name: Combined signal gen and feedback module
// Module Name: feedback_combined
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



module feedback_combined # 
(
    parameter ADC_DATA_WIDTH = 16,
    parameter DDS_OUT_WIDTH = 16,
    parameter PARAM_WIDTH = 32,
    parameter PARAM_A_OFFSET = 0,
    parameter PARAM_B_OFFSET = 32,
    parameter PARAM_C_OFFSET = 64,
    parameter PARAM_D_OFFSET = 96,
    parameter PARAM_E_OFFSET = 128,
    parameter PARAM_F_OFFSET = 160,
    parameter PARAM_G_OFFSET = 192,
    parameter PARAM_H_OFFSET = 224,
    parameter integer DAC_DATA_WIDTH = 14,
    parameter CFG_WIDTH = 256,
    parameter AXIS_TDATA_WIDTH = 16,
    parameter SELECT_WIDTH = 2,
    parameter CONTINUOUS_OUTPUT = 1
)
(
    input  wire                                 aclk,
    input  wire                                 trig_in,
    input [SELECT_WIDTH - 1:0]                  sel,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input [CFG_WIDTH-1:0]                       S_AXIS_CFG_tdata,
    input                                       S_AXIS_CFG_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input [ADC_DATA_WIDTH-1:0]                  S_AXIS_ADC1_tdata,
    input                                       S_AXIS_ADC1_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    input [ADC_DATA_WIDTH-1:0]                  S_AXIS_ADC2_tdata,
    input                                       S_AXIS_ADC2_tvalid,
    (* X_INTERFACE_PARAMETER = "FREQ_HZ 125000000" *)
    output reg [AXIS_TDATA_WIDTH-1:0]           M_AXIS_tdata,
    output wire                                 M_AXIS_tvalid,
    output reg                                  trig_out
);

////////////////////////
////Local Parameter////
///////////////////////

    //localparam PADDING_WIDTH = ADC_DATA_WIDTH - DAC_DATA_WIDTH;
    localparam RESULT_WIDTH = 64;
    localparam PHASE_WIDTH = 30;
    localparam PADDING_WIDTH = 32-ADC_DATA_WIDTH;

////////////////////    
////Declarations////
////////////////////

    // input registers
    reg signed [PARAM_WIDTH-1:0] Param_A_in, Param_B_in, Param_C_in, Param_D_in, Param_E_in, Param_F_in, Param_G_in, Param_H_in;
    reg signed [ADC_DATA_WIDTH-1:0] ADC1, ADC2;
       
    // state machine variables
    reg trigger;                       // 0 - trig output off, 1 - trig output on   
    reg [1:0] state;  
       
    parameter fixed = 0, sweep = 1, lin = 2, fancy = 3;    
    
    //Signals for counter
    reg counter_en = 1'b1;
    reg counter_nreset = 1'b1;
    reg [31:0] counter_interval = 32'b0;
    wire counter_thresh;
    
    //Signals for I/O from DDS
    reg [PHASE_WIDTH - 1:0] dds_phase_in; 
    wire signed [AXIS_TDATA_WIDTH - 1:0] dds_out;
    wire dds_M_AXIS_tvalid;
    
    // Math variables
    reg signed [RESULT_WIDTH - 1:0] result_A, result_A_last, result_B;
    reg signed [31:0] phase_next, phase = 32'b0;
    
    
////////////////////////////////////
////Instantiate further IP Cores////
////////////////////////////////////
    
    rollover_counter sweep_counter (
    .aclk(aclk),
    .en(counter_en),      
    .nRST(counter_nreset),  
    .MOD(counter_interval),
    .THRESH(counter_thresh)        
    ); 
    
    // Instantiate DDS compiler  
    dds_compiler_1 dds(
      .aclk(aclk),                                // input wire aclk
      .s_axis_phase_tvalid(S_AXIS_CFG_tvalid),  // input wire s_axis_phase_tvalid
      .s_axis_phase_tdata(dds_phase_in),    // input wire [31 : 0] s_axis_phase_tdata
      .m_axis_data_tvalid(dds_M_AXIS_tvalid),    // output wire m_axis_data_tvalid
      .m_axis_data_tdata(dds_out)      // output wire [15 : 0] m_axis_data_tdata
    );
    
//////////////////
////Data input////
//////////////////  

    // Bring in, break up, and store input data in registers as first pipeline stage.
    // All inputs stored, non-blocking, run in any state/
    always @(posedge aclk)
    begin
        ADC1 <= S_AXIS_ADC1_tdata;
        ADC2 <= S_AXIS_ADC2_tdata;
        state <= sel;
        trigger <= trig_in;
        //Configuration Parameters
        Param_A_in <= S_AXIS_CFG_tdata[PARAM_A_OFFSET + PARAM_WIDTH - 1: PARAM_A_OFFSET]; 
        Param_B_in <= S_AXIS_CFG_tdata[PARAM_B_OFFSET + PARAM_WIDTH - 1: PARAM_B_OFFSET]; 
        Param_C_in <= S_AXIS_CFG_tdata[PARAM_C_OFFSET + PARAM_WIDTH - 1: PARAM_C_OFFSET]; 
        Param_D_in <= S_AXIS_CFG_tdata[PARAM_D_OFFSET + PARAM_WIDTH - 1: PARAM_D_OFFSET]; 
        Param_E_in <= S_AXIS_CFG_tdata[PARAM_E_OFFSET + PARAM_WIDTH - 1: PARAM_E_OFFSET]; 
        Param_F_in <= S_AXIS_CFG_tdata[PARAM_F_OFFSET + PARAM_WIDTH - 1: PARAM_F_OFFSET]; 
        Param_G_in <= S_AXIS_CFG_tdata[PARAM_G_OFFSET + PARAM_WIDTH - 1: PARAM_G_OFFSET]; 
        Param_H_in <= S_AXIS_CFG_tdata[PARAM_H_OFFSET + PARAM_WIDTH - 1: PARAM_H_OFFSET];
        if (sel == 1)
            counter_interval <=  Param_B_in[31:31] ? (~Param_B_in+1'b1) : Param_B_in; //use absolut value of Param_B_in
        else
            counter_interval <= 32'b10000000;
    end

//////////// 
////Math////
////////////
    
     
    //Calculate output eqation parts
    always @(posedge aclk) 
    begin
        case (state)  
            fixed : result_A <= dds_out * Param_B_in;
            sweep : result_A <= dds_out * Param_C_in;
            lin : begin
                result_A <= ADC1 + Param_A_in;
                result_B <= result_A_last * Param_B_in;
            end
            default: result_A <= Param_B_in;
        endcase
    end 
    
    always @(negedge aclk) 
    begin
        case (state)  
            lin : result_A_last <= result_A;
        endcase
    end 
    
//////////////////////////
////Mux DDS, counter and output////
//////////////////////////
   
    // Mux input to DDS compiler - fixed phase or output from phase counter 
    always @(posedge aclk)
        case (state)
            fixed: dds_phase_in <= Param_A_in[29:0];
            sweep:
            begin
                if (~trigger) begin
                    phase_next <= Param_A_in;
                    dds_phase_in <= Param_A_in[29:0];
                    counter_nreset <= 0;
                    counter_en <= 0;
                end
                else begin 
                    if (counter_thresh)
                        phase_next <= (Param_B_in[31:31]) ? (phase - 1) :  (phase + 1); //add 1 if Param_B is positiv and -1 if negativ
                    dds_phase_in <= phase[29:0];
                    counter_nreset <= 1;
                    counter_en <= 1;
                end
            end
            default: dds_phase_in <= 0;
        endcase
        
    always @(negedge aclk)
    begin
        phase <= phase_next;
    end
    
    always @(*) 
    begin
        // Pass trigger through for output to external devices
        trig_out <= trigger;
        
        // Mux outpout on/off using trigger signal
        if (trigger || CONTINUOUS_OUTPUT)
        begin
            // Mux output depending on feedback state (output should be betwen 16'd8191 = 1V and -16'd8191 = -1V)
            case (state)  
                fixed : M_AXIS_tdata <= result_A[46:15]; //Take result devided by 8192 and shiftet further 2 bit for 14bit output
                sweep : M_AXIS_tdata <= result_A[46:15]; //Take result devided by 8192 and shiftet further 2 bit for 14bit output    
                lin :   M_AXIS_tdata <= result_B[43:12]; //Take result devided by 1024 and shiftet further 2 bit for 14bit output
                fancy:  M_AXIS_tdata <= result_A[31:0]; 
                default: M_AXIS_tdata <= 16'b0;
            endcase
         end    
    end                    
    
    //Valid output if input is valid
    assign M_AXIS_tvalid = S_AXIS_ADC1_tvalid & S_AXIS_ADC1_tvalid & S_AXIS_CFG_tvalid;// & dds_M_AXIS_tvalid;
    
endmodule