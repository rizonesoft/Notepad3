-- Simple Microprocessor Design (ESD Book Chapter 3)
-- Copyright 2001 Weijun Zhang
--
-- DATAPATH composed of Multiplexor, Register File and ALU
-- VHDL structural modeling
-- datapath.vhd
----------------------------------------------------------------

library	ieee;
use ieee.std_logic_1164.all;  
use ieee.std_logic_arith.all;			   
use ieee.std_logic_unsigned.all;

entity datapath is				
port(	clock_dp:	in 	std_logic;
	rst_dp:		in 	std_logic;
	imm_data:	in 	std_logic_vector(15 downto 0);
	mem_data: 	in 	std_logic_vector(15 downto 0);
	RFs_dp:		in 	std_logic_vector(1 downto 0);
	RFwa_dp:	in 	std_logic_vector(3 downto 0);
	RFr1a_dp:	in 	std_logic_vector(3 downto 0);
	RFr2a_dp:	in 	std_logic_vector(3 downto 0);
	RFwe_dp:	in 	std_logic;
	RFr1e_dp:	in 	std_logic;
	RFr2e_dp:	in 	std_logic;
	jp_en:		in 	std_logic;
	ALUs_dp:	in 	std_logic_vector(1 downto 0);
	oe_dp:		in 	std_logic;
	ALUz_dp:	out 	std_logic;
	RF1out_dp:	out 	std_logic_vector(15 downto 0);
	ALUout_dp:	out 	std_logic_vector(15 downto 0);
	bufout_dp:	out 	std_logic_vector(15 downto 0)
);
end datapath;

architecture struct of datapath is

component smallmux is
port( 	I0: 	in std_logic_vector(15 downto 0);
	I1: 	in std_logic_vector(15 downto 0);	  
	I2:	in std_logic_vector(15 downto 0);
	Sel:	in std_logic_vector(1 downto 0);
	O: 	out std_logic_vector(15 downto 0)
);
end component;

component reg_file is
port ( 	clock	: 	in std_logic;
	rst	: 	in std_logic;
	RFwe	: 	in std_logic;
	RFr1e	: 	in std_logic;
	RFr2e	: 	in std_logic;
	RFwa	: 	in std_logic_vector(3 downto 0);
	RFr1a	: 	in std_logic_vector(3 downto 0);
	RFr2a	: 	in std_logic_vector(3 downto 0);
	RFw	: 	in std_logic_vector(15 downto 0);
	RFr1	: 	out std_logic_vector(15 downto 0);
	RFr2	:	out std_logic_vector(15 downto 0)
);
end component;

component alu is
port (	num_A: 	in std_logic_vector(15 downto 0);
	num_B: 	in std_logic_vector(15 downto 0);
	jpsign:	in std_logic;
	ALUs:	in std_logic_vector(1 downto 0);
	ALUz:	out std_logic;
	ALUout:	out std_logic_vector(15 downto 0)
);
end component;

component obuf is
port(	O_en: 		in std_logic;
	obuf_in: 	in std_logic_vector(15 downto 0);
	obuf_out: 	out std_logic_vector(15 downto 0)
);
end component;

signal mux2rf, rf2alu1: std_logic_vector(15 downto 0); 
signal rf2alu2, alu2memmux: std_logic_vector(15 downto 0);

begin		

  U1: smallmux port map(alu2memmux, mem_data, 
			imm_data, RFs_dp, mux2rf);
  U2: reg_file port map(clock_dp, rst_dp, RFwe_dp, 
			RFr1e_dp, RFr2e_dp, 
			RFwa_dp, RFr1a_dp, RFr2a_dp, 
			mux2rf, rf2alu1, rf2alu2 );
  U3: alu port map( rf2alu1, rf2alu2, jp_en, ALUs_dp, 
		    ALUz_dp, alu2memmux);
  U4: obuf port map(oe_dp, mem_data, bufout_dp);
	
  ALUout_dp <= alu2memmux;
  RF1out_dp <= rf2alu1;
	
end struct;



--------------------------------------------------------
-- Simple Microprocessor Design
--
-- memory 256*16
-- 8 bit address; 16 bit data
-- memory.vhd
--------------------------------------------------------

library	ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;   
use work.constant_lib.all;

entity memory is
port ( 	clock	: 	in std_logic;
		rst		: 	in std_logic;
		Mre		:	in std_logic;
		Mwe		:	in std_logic;
		address	:	in std_logic_vector(7 downto 0);
		data_in	:	in std_logic_vector(15 downto 0);
		data_out:	out std_logic_vector(15 downto 0)
);
end memory;

architecture behv of memory is			

  type ram_type is array (0 to 255) of 
        		std_logic_vector(15 downto 0);
  signal tmp_ram: ram_type;

begin
	

							
	write: process(clock, rst, Mre, address, data_in)
	begin
		if rst='1' then		
			tmp_ram <= (------------------------------------------------------------ 
						-- following instruction for debugging purpose
						-- 0 => "0011001000101011",			   	-- R2 <= #2B
						-- 1 => "0011001100001001",			   	-- R3 <= #09
						-- 2 => "0001001100001010",			   	-- M[A] <= R3
						-- 3 => "0000011000001000",			   	-- R6 <= M[8]
						-- 4 => "0010001100100000",				-- M[R3] <= R2
						-- 5 => "0100001100100000",				-- R3 <= R3 + R2
						-- 6 => "0101001001100000",				-- R2 <= R2 - R6
						-- 7 => "0110000000000000",				-- jz if R0=0
						-- 8 => "1111000000000000",				-- halt	
						-------------------------------------------------------------
						
						-- program to generate 10 fabonacci number	  
							
						0 => "0011000000000000",	   		-- mov R0, #0
						1 => "0011000100000001",			-- mov R1, #1
						2 => "0011001000110100",			-- mov R2, #52
						3 => "0011001100000001",			-- mov R3, #1
						4 => "0001000000110010",			-- mov M[50], R0
						5 => "0001000100110011",			-- mov M[51], R1
						6 => "0001000101100100",			-- mov M[100], R1
						7 => "0100000100000000",			-- add R1, R0
						8 => "0000000001100100",			-- mov M[100], R0
						9 => "0010001000010000",			-- mov M[R2], R1
						10 => "0100001000110000",			-- add R2, R3
						11 => "0000010000111011",			-- mov R4, M[59]
						12 => "0110010000000101",  			-- jz R4, #6
						
						13 => "0111000000110010",			-- output  M[50]
						14 => "0111000000110011",			--		,, M[51]
						15 => "0111000000110100",			--		,, M[52]
						16 => "0111000000110101",			--		,, M[53]
						17 => "0111000000110110",			--		,, M[54]
						18 => "0111000000110111",			--		,, M[55]
						19 => "0111000000111000",			--		,, M[56]
						20 => "0111000000111001",			--		,, M[57]
						21 => "0111000000111010",			--		,, M[58]
						22 => "0111000000111011",			--		,, M[59]
												
						23 => "1111000000000000",			-- halt
						others => "0000000000000000");
		else
			if (clock'event and clock = '1') then
				if (Mwe ='1' and Mre = '0') then
					tmp_ram(conv_integer(address)) <= data_in;
				end if;
			end if;
		end if;
	end process;

    read: process(clock, rst, Mwe, address)
	begin
		if rst='1' then
			data_out <= ZERO;
		else
			if (clock'event and clock = '1') then
				if (Mre ='1' and Mwe ='0') then								 
					data_out <= tmp_ram(conv_integer(address));
				end if;
			end if;
		end if;
	end process;
end behv;


--------------------------------------------------------
-- Simple Microprocessor Design (ESD Book Chapter 3)
-- Copyright 2001 Weijun Zhang
--
-- big multiplexor of control unit has
-- four 16-bit inputs and one 16-bit output
-- bigmux.vhd
--------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity bigmux is
port( 	Ia: 	in std_logic_vector(15 downto 0);
	Ib: 	in std_logic_vector(15 downto 0);	  
	Ic:	in std_logic_vector(15 downto 0);
	Id:	in std_logic_vector(15 downto 0);
	Option:	in std_logic_vector(1 downto 0);
	Muxout:	out std_logic_vector(15 downto 0)
);
end bigmux;

architecture behv of bigmux is

begin
    
    process(Ia, Ib, Ic, Id, Option)
    begin
      case Option is
	when "00" => Muxout <= Ia;
        when "01" => Muxout <= Ib;
	when "10" => Muxout <= Ic; 
	when "11" => Muxout <= Id;
	when others =>  
      end case;
    end process;

end behv;

