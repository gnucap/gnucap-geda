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

module POLARIZED_CAPACITOR(p n);
endmodule

module RESISTOR(1 2);
	parameter value=1;
	resistor #(.r(value)) r(1,2);
endmodule

module BATTERY(n p);
	parameter value=1;
	vsource #(.dc(value)) v(n,p);
endmodule

module VOLTAGE_SOURCE(1 2);
	parameter value=1;
	vsource #(.dc(value)) v(1 2);
endmodule

module CURRENT_SOURCE(1 2);
	parameter value=1;
	isource #(.dc(value)) i(p n);
endmodule

module BC547(1 2 3 4);
// nothing yet
endmodule

module TRANSFORMER(1 2 3 4 5 6 7);
// not yet
endmodule

module NMOS_TRANSISTOR(D G S B);
// not yet
endmodule

module PMOS_TRANSISTOR(D G S B);
// not yet
endmodule

module NPN_TRANSISTOR(C B E);
// later..
endmodule

module LED(CATHODE, ANODE);
// nothing
endmodule

simulator lang=acs
