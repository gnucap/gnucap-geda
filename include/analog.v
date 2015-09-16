simulator language=verilog
// This File is part of gnucap-geda
// (C) 2012 Felix Salfelder
// GPLv3 or later

// mapping geda-symbols to actual devices
// "analog" section

module CAPACITOR(1 2);
	parameter value=1;
	capacitor #(.c(value)) c(1,2);
endmodule

module RESISTOR(1 2);
	parameter value=1;
	resistor #(.r(value)) r(1,2);
endmodule

module BATTERY(n p);
	parameter value=1;
	vsource #(.dc(value)) v(n,p);
endmodule

module VOLTAGE_SOURCE(p n);
	parameter value=1;
	vsource #(.dc(value)) v(p n);
endmodule

module CURRENT_SOURCE(1 2);
	parameter value=1;
	isource #(.dc(value)) i(p n);
endmodule

module BC547(1 2 3 4);
// nothing yet
endmodule

simulator lang=acs
