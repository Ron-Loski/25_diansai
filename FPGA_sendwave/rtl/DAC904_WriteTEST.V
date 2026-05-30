module DAC904_WriteTEST(
	input clk,
	input rstn,
	input mcu_uart_rx,

	output clk_driver,		// DAC时钟输出
	output [13:0]IO_data		// DAC数据输出
	);

	wire areset, pll165m_lockedPIN, pll_65M, pll_165M;
	assign areset = ~rstn;

	// 锁相环：输入时钟倍频产生 66MHz 和 165MHz
	pll165m	U0_pll165m_inst (
		.areset ( areset ),
		.inclk0 ( clk ),
		.c0 ( pll_65M ),			// 66MHz
		.c1 ( pll_165M ),			// 165MHz
		.locked ( pll165m_lockedPIN )
		);

	// ============================================================
	// 在 "rom_14B_Sin.v" 中，取消注释其中一行 init_file 即可切换波形
	// 改完后重新编译，下载到FPGA，用示波器可观察到DAC输出波形：
	//
	//   ROM_14B_SQU.mif    — 方波
	//   ROM_14B_TRIANG.mif — 三角波
	//   ROM_14B_SIN.mif    — 正弦波（当前选择）
	//
	// \rtl\ROM\ROM_14B_SIN\ 路径下的 MIF 文件存放自定义波形数据
	// 也可根据需要自行编辑
	// ============================================================

	// DDS 频率控制字（165MSPS, 32位相位累加器）
	// 选择其中一个赋值给 U1 的 PhaseStep，即可获得对应输出频率
	parameter freCtrlWord_1k   = 32'd26030;
	parameter freCtrlWord_100k = 32'd2603010;
	parameter freCtrlWord_1M   = 32'd26030104;
	parameter freCtrlWord_10M  = 32'd260301048;
	parameter freCtrlWord_20M  = 32'd520602096;
	parameter freCtrlWord_30M  = 32'd780903144;
	parameter freCtrlWord_40M  = 32'd1041204192;
	parameter freCtrlWord_50M  = 32'd1301505241;

	wire [7:0] uart_rx_data;
	wire uart_rx_done;

	UartRx #(
		.CLKS_PER_BIT ( 16'd217 )
	) U4_UartRx (
		.clk      ( clk ),
		.rstn     ( rstn ),
		.uart_rx  ( mcu_uart_rx ),
		.rx_data  ( uart_rx_data ),
		.rx_done  ( uart_rx_done )
	);

	reg [2:0] uart_frame_index;
	reg [7:0] uart_cmd;
	reg [31:0] uart_data;
	reg [7:0] uart_xor;

	reg [31:0] phase_step_ctrl;
	reg wave_enable_ctrl;
	reg [10:0] amp_scale_ctrl;
	reg ctrl_update_toggle;

	always @(posedge clk or negedge rstn) begin
		if (!rstn) begin
			uart_frame_index <= 3'd0;
			uart_cmd <= 8'd0;
			uart_data <= 32'd0;
			uart_xor <= 8'd0;
			phase_step_ctrl <= freCtrlWord_100k;
			wave_enable_ctrl <= 1'b1;
			amp_scale_ctrl <= 11'd1024;
			ctrl_update_toggle <= 1'b0;
		end
		else if (uart_rx_done) begin
			case (uart_frame_index)
				3'd0: begin
					if (uart_rx_data == 8'hA5) begin
						uart_frame_index <= 3'd1;
						uart_xor <= 8'hA5;
					end
				end

				3'd1: begin
					uart_cmd <= uart_rx_data;
					uart_xor <= uart_xor ^ uart_rx_data;
					uart_frame_index <= 3'd2;
				end

				3'd2: begin
					uart_data[7:0] <= uart_rx_data;
					uart_xor <= uart_xor ^ uart_rx_data;
					uart_frame_index <= 3'd3;
				end

				3'd3: begin
					uart_data[15:8] <= uart_rx_data;
					uart_xor <= uart_xor ^ uart_rx_data;
					uart_frame_index <= 3'd4;
				end

				3'd4: begin
					uart_data[23:16] <= uart_rx_data;
					uart_xor <= uart_xor ^ uart_rx_data;
					uart_frame_index <= 3'd5;
				end

				3'd5: begin
					uart_data[31:24] <= uart_rx_data;
					uart_xor <= uart_xor ^ uart_rx_data;
					uart_frame_index <= 3'd6;
				end

				3'd6: begin
					if (uart_rx_data == uart_xor) begin
						if (uart_cmd == 8'h01) begin
							phase_step_ctrl <= uart_data;
							ctrl_update_toggle <= ~ctrl_update_toggle;
						end
						else if (uart_cmd == 8'h02) begin
							wave_enable_ctrl <= uart_data[0];
							ctrl_update_toggle <= ~ctrl_update_toggle;
						end
						else if (uart_cmd == 8'h03) begin
							if (uart_data > 32'd1024)
								amp_scale_ctrl <= 11'd1024;
							else
								amp_scale_ctrl <= uart_data[10:0];
							ctrl_update_toggle <= ~ctrl_update_toggle;
						end
					end
					uart_frame_index <= 3'd0;
				end

				default: begin
					uart_frame_index <= 3'd0;
				end
			endcase
		end
	end

	reg [2:0] ctrl_update_sync;
	reg [31:0] phase_step_dds;
	reg wave_enable_dds;
	reg [10:0] amp_scale_dds;

	always @(posedge pll_165M or negedge rstn) begin
		if (!rstn) begin
			ctrl_update_sync <= 3'd0;
			phase_step_dds <= freCtrlWord_100k;
			wave_enable_dds <= 1'b1;
			amp_scale_dds <= 11'd1024;
		end
		else begin
			ctrl_update_sync <= {ctrl_update_sync[1:0], ctrl_update_toggle};
			if (ctrl_update_sync[2] ^ ctrl_update_sync[1]) begin
				phase_step_dds <= phase_step_ctrl;
				wave_enable_dds <= wave_enable_ctrl;
				amp_scale_dds <= amp_scale_ctrl;
			end
		end
	end

	wire [13:0]romAddr;

	// PhaseAccumulator: DDS 相位累加器
	//   CLK       — 输入，时钟 (165MHz)
	//   RSTn      — 输入，复位（低有效）
	//   PhaseStep — 输入，频率控制字
	//   MemAddr   — 输出，ROM 地址
	//   公式：PhaseStep = 输出频率 / 165M * (2^32)
	//   例如：1MHz → 1000000 / 165000000 * (2^32) ≈ 26030104
	PhaseAccumulator U1_PhaseAccumulator(
	   .CLK      ( pll_165M ),
	   .RSTn     ( rstn ),
	   .PhaseStep( phase_step_dds ),
	   .MemAddr  ( romAddr )
	);

	wire[13:0] dacData;

	// rom_14B_Sin: 波形查找表 ROM
	//   address — 输入，ROM 读地址
	//   clock   — 输入，时钟
	//   q       — 输出，波形采样值
	rom_14B_Sin	U2_rom_14B_Sin(
		.address ( romAddr ),
		.clock   ( pll_165M ),
		.q       ( dacData )
		);

	assign clk_driver = pll_165M;
	wire [13:0]w_IO_Data;
	wire [13:0]dacOutputData;
	reg signed [14:0] dacCenteredReg;
	reg signed [26:0] dacMultReg;
	reg signed [16:0] dacScaledReg;
	reg signed [16:0] dacSumReg;
	reg [13:0] dacAmpData;

	always @(posedge pll_165M or negedge rstn) begin
		if (!rstn) begin
			dacCenteredReg <= 15'sd0;
			dacMultReg <= 27'sd0;
			dacScaledReg <= 17'sd0;
			dacSumReg <= 17'sd8192;
			dacAmpData <= 14'd8192;
		end
		else begin
			dacCenteredReg <= $signed({1'b0, dacData}) - 15'sd8192;
			dacMultReg <= dacCenteredReg * $signed({1'b0, amp_scale_dds});
			dacScaledReg <= dacMultReg[26:10];

			if (dacScaledReg > 17'sd8191)
				dacSumReg <= 17'sd16383;
			else if (dacScaledReg < -17'sd8192)
				dacSumReg <= 17'sd0;
			else
				dacSumReg <= dacScaledReg + 17'sd8192;

			if (dacSumReg > 17'sd16383)
				dacAmpData <= 14'd16383;
			else if (dacSumReg < 17'sd0)
				dacAmpData <= 14'd0;
			else
				dacAmpData <= dacSumReg[13:0];
		end
	end

	assign dacOutputData = wave_enable_dds ? dacAmpData : 14'd8192;

	// DAC904_WriteModule: 数据锁存后输出到 DAC904 管脚
	//   DAC_CLKneg — 输入，时钟（下降沿）
	//   RSTn       — 输入，复位（低有效）
	//   inputdata  — 输入，波形采样值
	//   outputData — 输出，接 DAC904 数据管脚
	DAC904_WriteModule U3_DAC904_WriteModule(
		 .DAC_CLKneg( pll_165M ),
		 .RSTn      ( rstn ),
		 .inputdata ( dacOutputData ),
		 .outputData( IO_data )
	);

	endmodule
