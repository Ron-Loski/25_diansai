module UartRx
#(
    parameter CLKS_PER_BIT = 16'd217
)
(
    input clk,
    input rstn,
    input uart_rx,

    output reg [7:0] rx_data,
    output reg rx_done
);

localparam S_IDLE  = 2'd0;
localparam S_START = 2'd1;
localparam S_DATA  = 2'd2;
localparam S_STOP  = 2'd3;
localparam [15:0] BIT_CLKS = CLKS_PER_BIT;
localparam [15:0] HALF_BIT_CLKS = CLKS_PER_BIT >> 1;

reg [1:0] state;
reg [15:0] clk_cnt;
reg [2:0] bit_cnt;
reg [7:0] rx_shift;
reg uart_rx_meta;
reg uart_rx_sync;

always @(posedge clk or negedge rstn) begin
    if (!rstn) begin
        uart_rx_meta <= 1'b1;
        uart_rx_sync <= 1'b1;
    end
    else begin
        uart_rx_meta <= uart_rx;
        uart_rx_sync <= uart_rx_meta;
    end
end

always @(posedge clk or negedge rstn) begin
    if (!rstn) begin
        state <= S_IDLE;
        clk_cnt <= 16'd0;
        bit_cnt <= 3'd0;
        rx_shift <= 8'd0;
        rx_data <= 8'd0;
        rx_done <= 1'b0;
    end
    else begin
        rx_done <= 1'b0;

        case (state)
            S_IDLE: begin
                clk_cnt <= 16'd0;
                bit_cnt <= 3'd0;
                if (!uart_rx_sync) begin
                    state <= S_START;
                    clk_cnt <= HALF_BIT_CLKS;
                end
            end

            S_START: begin
                if (clk_cnt == 16'd0) begin
                    if (!uart_rx_sync) begin
                        state <= S_DATA;
                        clk_cnt <= BIT_CLKS - 16'd1;
                    end
                    else begin
                        state <= S_IDLE;
                    end
                end
                else begin
                    clk_cnt <= clk_cnt - 16'd1;
                end
            end

            S_DATA: begin
                if (clk_cnt == 16'd0) begin
                    rx_shift[bit_cnt] <= uart_rx_sync;
                    clk_cnt <= BIT_CLKS - 16'd1;

                    if (bit_cnt == 3'd7) begin
                        bit_cnt <= 3'd0;
                        state <= S_STOP;
                    end
                    else begin
                        bit_cnt <= bit_cnt + 3'd1;
                    end
                end
                else begin
                    clk_cnt <= clk_cnt - 16'd1;
                end
            end

            S_STOP: begin
                if (clk_cnt == 16'd0) begin
                    if (uart_rx_sync) begin
                        rx_data <= rx_shift;
                        rx_done <= 1'b1;
                    end
                    state <= S_IDLE;
                end
                else begin
                    clk_cnt <= clk_cnt - 16'd1;
                end
            end

            default: begin
                state <= S_IDLE;
            end
        endcase
    end
end

endmodule
