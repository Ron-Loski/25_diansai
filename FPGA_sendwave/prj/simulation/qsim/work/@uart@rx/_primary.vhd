library verilog;
use verilog.vl_types.all;
entity UartRx is
    port(
        clk             : in     vl_logic;
        rstn            : in     vl_logic;
        uart_rx         : in     vl_logic;
        rx_data         : out    vl_logic_vector(7 downto 0);
        rx_done         : out    vl_logic
    );
end UartRx;
