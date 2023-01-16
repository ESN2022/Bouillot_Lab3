library ieee;
use ieee.std_logic_1164.all;

entity bin_to_7seg is

  port (input: in std_logic_vector(3 downto 0);
        output: out std_logic_vector(7 downto 0)
		  );
end entity;

architecture arch of bin_to_7seg is
begin
	output <= 	not("00111111") when input="0000" else		--"0"
					not("00000110") when input="0001" else		--"1"
					not("01011011") when input="0010" else		--"2"
					not("01001111") when input="0011" else		--"3"
					not("01100110") when input="0100" else		--"4"
					not("01101101") when input="0101" else		--"5"
					not("01111101") when input="0110" else		--"6"
					not("00000111") when input="0111" else		--"7"
					not("01111111") when input="1000" else		--"8"
					not("01101111") when input="1001" else		--"9"
					
					not("01000000") when input="1010" else		--signe négatif -
					not("00000000") when input="1011" else		--éteint
					
					not("01111111");
end architecture;