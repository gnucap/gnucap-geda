simulator language=verilog
// This File is part of gnucap-geda
// (C) 2012 Felix Salfelder
// GPLv3 or later

// mapping geda-symbols to actual devices
// "analog" section

module CAPACITOR(n p);
	parameter value=1;
	capacitor #(.c(value)) c(n,p);
endmodule

module RESISTOR(n p);
	parameter value=1;
	resistor #(.r(value)) r(n,p);
endmodule

module BATTERY(n p);
	parameter value=1;
	vsource #(.dc(value)) v(n,p);
endmodule

simulator lang=acs
