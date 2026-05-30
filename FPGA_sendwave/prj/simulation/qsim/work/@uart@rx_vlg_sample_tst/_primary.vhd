library verilog;
use verilog.vl_types.all;
entity UartRx_vlg_sample_tst is
    port(
        clk             : in     vl_logic;
        rstn            : in     vl_logic;
        uart_rx         : in     vl_logic;
        sampler_tx      : out    vl_logic
    );
end UartRx_vlg_sample_tst;
