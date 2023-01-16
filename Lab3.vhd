library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


entity lab3 is
   port(
      clk	  			: in  std_logic;
      reset	  			: in  std_logic;
		push_b			: in std_logic;
		output0 			: out std_logic_vector(7 downto 0);
		output1 			: out std_logic_vector(7 downto 0);
		output2 			: out std_logic_vector(7 downto 0);
		output3 			: out std_logic_vector(7 downto 0);
		output4 			: out std_logic_vector(7 downto 0);
		output5 			: out std_logic_vector(7 downto 0);
		GSENSOR_CS_n 	: out std_logic;			--active at low level
		GSENSOR_SDO 	: out std_logic;
		i2c_SDA 			: inout std_logic ;
		i2c_SCL 			: inout std_logic
      );
end entity;


architecture arch of lab3 is

component lab3_qsys is
	  port (
			clk_clk                          	: in  std_logic                     	:= 'X';
			opencores_i2c_0_export_0_scl_pad_io : inout std_logic                		:= 'X';
         opencores_i2c_0_export_0_sda_pad_io : inout std_logic                     	:= 'X'; 
			pio_0_external_connection_export 	: out std_logic_vector(23 downto 0)				;
			pio_1_external_connection_export    : in    std_logic                     	:= 'X';
			reset_reset_n                    	: in  std_logic                     	:= 'X'
	  );
end component lab3_qsys;

component bin_to_7seg is
  port (
		input                          	: in  std_logic_vector(3 downto 0);
		output 									: out std_logic_vector(7 downto 0)
  );
end component bin_to_7seg;

signal in_signal : std_logic_vector(23 downto 0);

begin
	GSENSOR_CS_n 	<= 	'1';		--mode de communication en I2C
	GSENSOR_SDO 	<= 	'1';		

    u0 : component lab3_qsys
        port map (
            clk_clk                          	=> clk,
				opencores_i2c_0_export_0_scl_pad_io => i2c_SCL,
				opencores_i2c_0_export_0_sda_pad_io => i2c_SDA,
            pio_0_external_connection_export 	=> in_signal,
				pio_1_external_connection_export    => push_b,
            reset_reset_n                    	=> reset
    );

	u1: component bin_to_7seg
	port map (
		input                          	=> in_signal(3 downto 0),                         
		output 									=> output0                
  );

   u2: component bin_to_7seg
	port map (
		input                          	=> in_signal(7 downto 4),                         
		output 									=> output1                  
  );

   u3: component bin_to_7seg
	port map (
		input                          	=> in_signal(11 downto 8),                         
		output 									=> output2                  
  );

  	u4: component bin_to_7seg
	port map (
		input                          	=> in_signal(15 downto 12),                         
		output 									=> output3                  
  );

   u5: component bin_to_7seg
	port map (
		input                          	=> in_signal(19 downto 16),                         
		output 									=> output4                  
  );

   u6: component bin_to_7seg
	port map (
		input                          	=> in_signal(23 downto 20),                         
		output 									=> output5                  
  );


end architecture;