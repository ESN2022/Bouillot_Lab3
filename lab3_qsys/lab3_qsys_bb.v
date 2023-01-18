
module lab3_qsys (
	clk_clk,
	opencores_i2c_0_export_0_scl_pad_io,
	opencores_i2c_0_export_0_sda_pad_io,
	pio_0_external_connection_export,
	pio_1_external_connection_export,
	reset_reset_n);	

	input		clk_clk;
	inout		opencores_i2c_0_export_0_scl_pad_io;
	inout		opencores_i2c_0_export_0_sda_pad_io;
	output	[23:0]	pio_0_external_connection_export;
	input		pio_1_external_connection_export;
	input		reset_reset_n;
endmodule
