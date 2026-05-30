#------------------GLOBAL--------------------#
set_global_assignment -name RESERVE_ALL_UNUSED_PINS "AS INPUT TRI-STATED"
set_global_assignment -name ENABLE_INIT_DONE_OUTPUT OFF

# system clock (50MHz onboard)
set_location_assignment	PIN_M2	-to clk

# system reset (active low)
set_location_assignment	PIN_M1	-to rstn

# MCU UART control input
set_location_assignment	PIN_A8	-to mcu_uart_rx

# DAC clock output
set_location_assignment	PIN_E6	-to clk_driver

# DAC data outputs (14-bit)
set_location_assignment	PIN_K6	-to IO_data[0]
set_location_assignment	PIN_D3	-to IO_data[1]
set_location_assignment	PIN_D5	-to IO_data[2]
set_location_assignment	PIN_B13	-to IO_data[3]
set_location_assignment	PIN_D6	-to IO_data[4]
set_location_assignment	PIN_E7	-to IO_data[5]
set_location_assignment	PIN_A13	-to IO_data[6]
set_location_assignment	PIN_M7	-to IO_data[7]
set_location_assignment	PIN_E8	-to IO_data[8]
set_location_assignment	PIN_M6	-to IO_data[9]
set_location_assignment	PIN_N5	-to IO_data[10]
set_location_assignment	PIN_N11	-to IO_data[11]
set_location_assignment	PIN_T12	-to IO_data[12]
set_location_assignment	PIN_M11	-to IO_data[13]

# unused legacy key inputs (not in top module)
set_location_assignment	PIN_K15	-to keyIn[0]
set_location_assignment	PIN_L15	-to keyIn[1]
set_location_assignment	PIN_L14	-to keyIn[2]
set_location_assignment	PIN_N15	-to keyIn[3]
set_location_assignment	PIN_P15	-to keyIn[4]

# unused legacy key input
set_location_assignment	PIN_J16	-to keyAF
