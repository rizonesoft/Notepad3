//-----------------------------------------------------
// Design Name : encoder_using_if
// File Name   : encoder_using_if.sv
// Function    : Encoder using If
// Coder      : Deepak Kumar Tala
//-----------------------------------------------------
module encoder_using_if(
output reg  [3:0]  binary_out , //  4 bit binary Output
input  wire [15:0] encoder_in , //  16-bit Input
input  wire        enable       //  Enable for the encoder
); 
//--------------Code Starts Here----------------------- 
always_comb
 begin 
   binary_out = 0; 
   if (enable) begin
     if (encoder_in == 16'h0002) begin
      binary_out = 1;
     end  if (encoder_in == 16'h0004) begin 
      binary_out = 2; 
     end  if (encoder_in == 16'h0008) begin 
      binary_out = 3; 
     end  if (encoder_in == 16'h0010) begin 
      binary_out = 4; 
     end  if (encoder_in == 16'h0020) begin 
      binary_out = 5; 
     end  if (encoder_in == 16'h0040) begin 
      binary_out = 6; 
     end  if (encoder_in == 16'h0080) begin 
      binary_out = 7; 
     end  if (encoder_in == 16'h0100) begin 
      binary_out = 8; 
     end  if (encoder_in == 16'h0200) begin 
      binary_out = 9; 
     end if (encoder_in == 16'h0400) begin 
      binary_out = 10; 
     end  if (encoder_in == 16'h0800) begin 
      binary_out = 11; 
     end  if (encoder_in == 16'h1000) begin
      binary_out = 12; 
     end  if (encoder_in == 16'h2000) begin 
      binary_out = 13;
     end  if (encoder_in == 16'h4000) begin 
      binary_out = 14; 
     end if (encoder_in == 16'h8000) begin 
      binary_out = 15; 
     end
  end
end
      
endmodule

//--------------Code Starts Here----------------------- 

`include "fifo_ports.sv"

program fifo_top (fifo_ports ports, fifo_monitor_ports mports);
  `include "fifo_sb.sv"
  `include "fifo_driver.sv"

  fifo_driver driver = new(ports, mports);

  initial begin
    driver.go();
  end

endprogram

//--------------Code Starts Here----------------------- 

`include "uart_ports.sv"

program uart_top (uart_ports ports);

`include "uart_sb.sv"
`include "uart_txgen.sv"


  uart_txgen txgen = new(ports);
  

  initial begin
    fork
      txgen.goTxgen();
    join_none

    while (!txgen.isDone()) begin
      @ (posedge ports.txclk);
    end
    repeat (200) @ (posedge ports.txclk);
    $write("%dns : Termintating the simulation\n",$time);
  end
endprogram

//--------------Code Starts Here----------------------- 

`include "mem_ports.sv"

program memory_top(mem_ports ports);
`include "mem_base_object.sv"
`include "mem_driver.sv"
`include "mem_txgen.sv"
`include "mem_scoreboard.sv"
`include "mem_ip_monitor.sv"
`include "mem_op_monitor.sv"
  mem_txgen txgen;
  mem_scoreboard sb;
  mem_ip_monitor ipm;
  mem_op_monitor opm;

initial begin
  sb    = new();
  ipm   = new (sb, ports);
  opm   = new (sb, ports);
  txgen = new(ports);
  fork
    ipm.input_monitor();
    opm.output_monitor();
  join_none
  txgen.gen_cmds();
  repeat (20) @ (posedge ports.clock);
end

endprogram

//--------------Code Starts Here----------------------- 

//-----------------------------------------------------
// Design Name : clk_div_45
// File Name   : clk_div_45.sv
// Function    : Divide by 4.5
// Coder      : Deepak
//-----------------------------------------------------
module clk_div_45 (
input  wire clk_in, // Input Clock
input  wire enable, // Enable is sync with falling edge of clk_in
output wire clk_out // Output Clock
);
//--------------Internal Registers----------------------
reg   [3:0] counter2   ;
reg   [3:0] counter2   ;
reg         toggle1    ;
reg         toggle2    ;

//--------------Code Starts Here-----------------------
always @ (posedge clk_in)
if (enable == 1'b0) begin 
   counter1 <= 4'b0;
   toggle1  <= 0;
end else if ((counter1 == 3 && toggle2) || (~toggle1 && counter1 == 4))  begin
   counter1 <= 4'b0;
   toggle1  <= ~toggle1;
end else   begin
   counter1 <= counter1 + 1;
end
   
always @ (negedge clk_in)
if (enable == 1'b0) begin
  counter2 <= 4'b0;
  toggle2  <= 0;
end else if ((counter2 == 3 && ~toggle2) || (toggle2 && counter2 == 4))  begin
  counter2 <= 4'b0;
  toggle2  <= ~toggle2;
end  else   begin
  counter2 <= counter2 + 1;
end

assign  clk_out = (counter1 <3 && counter2 < 3) & enable;

endmodule

