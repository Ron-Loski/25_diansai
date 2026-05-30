onerror {exit -code 1}
vlib work
vlog -work work DAC904_HighSpeedDAC.vo
vlog -work work uart.vwf.vt
vsim -novopt -c -t 1ps -L cycloneive_ver -L altera_ver -L altera_mf_ver -L 220model_ver -L sgate work.UartRx_vlg_vec_tst -voptargs="+acc"
vcd file -direction DAC904_HighSpeedDAC.msim.vcd
vcd add -internal UartRx_vlg_vec_tst/*
vcd add -internal UartRx_vlg_vec_tst/i1/*
run -all
quit -f
