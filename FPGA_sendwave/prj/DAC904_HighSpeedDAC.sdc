
# 50MHz system clock
create_clock -name clk_50m -period 20.000 [get_ports clk]

# Derive PLL output clocks automatically (165MHz / 66MHz)
derive_pll_clocks

# UART input (slow signal, relax constraint)
set_input_delay -clock clk_50m -max 20.0 [get_ports mcu_uart_rx]
set_input_delay -clock clk_50m -min 2.0  [get_ports mcu_uart_rx]

# Reset input
set_input_delay -clock clk_50m -max 10.0 [get_ports rstn]
set_input_delay -clock clk_50m -min 2.0  [get_ports rstn]

# CDC: UART control registers are slow configuration values transferred to the
# 165MHz DDS domain by ctrl_update_toggle. Do not time multi-bit payload paths
# across clock domains; only the toggle synchronizer is timed.
set_false_path -from [get_registers {*phase_step_ctrl* *wave_enable_ctrl* *amp_scale_ctrl*}] -to [get_registers {*phase_step_dds*  *wave_enable_dds*  *amp_scale_dds*}]
