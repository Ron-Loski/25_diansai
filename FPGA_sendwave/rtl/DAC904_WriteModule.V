module DAC904_WriteModule(
	 DAC_CLKneg,		// 输入，时钟（下降沿）
	 RSTn,			    // 输入，复位（低有效）
	 inputdata, 		    // 输入，波形采样值

	 outputData		    // 输出，接 DAC904 数据管脚
	);

	input RSTn;
	input[13:0] inputdata;
	input DAC_CLKneg;

	output[13:0] outputData;

	reg[13:0] rOutputData;
	always @ ( negedge DAC_CLKneg or negedge RSTn )
	if( !RSTn )
	   rOutputData <= 14'd0;
	else
	   rOutputData <= inputdata;

	assign outputData = rOutputData;

	endmodule
