// PhaseAccumulator: DDS 相位累加器
//   CLK       — 输入，时钟
//   RSTn      — 输入，复位（低有效）
//   PhaseStep — 输入，频率控制字
//   MemAddr   — 输出，取累加器高 14 位作为 ROM 寻址地址

module PhaseAccumulator(
   CLK,
   RSTn,

   PhaseStep,
   MemAddr
   );

`define PhaseAcc_BITS_NUM  32
`define Mem_BITS_NUM        14

 input CLK;
 input RSTn;
 input [`PhaseAcc_BITS_NUM-1:0] PhaseStep;
 output [`Mem_BITS_NUM-1:0]MemAddr;

reg[`PhaseAcc_BITS_NUM-1:0] PhaseSum;
always @ ( posedge CLK or negedge RSTn)
if( !RSTn ) begin
    PhaseSum <= 32'd0;
end
else
begin
    PhaseSum <= PhaseSum + PhaseStep;
end

// 取相位累加器的高 Mem_BITS_NUM 位作为 ROM 地址
assign MemAddr = {PhaseSum[`PhaseAcc_BITS_NUM-1:`PhaseAcc_BITS_NUM-`Mem_BITS_NUM]};

endmodule
