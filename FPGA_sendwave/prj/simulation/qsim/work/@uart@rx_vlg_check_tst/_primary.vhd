library verilog;
use verilog.vl_types.all;
entity UartRx_vlg_check_tst is
    port(
        rx_data         : in     vl_logic_vector(7 downto 0);
        rx_done         : in     vl_logic;
        sampler_rx      : in     vl_logic
    );
end UartRx_vlg_check_tst;
