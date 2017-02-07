simulator language=verilog
// This File is part of gnucap-geda
// (C) 2012 Felix Salfelder
// GPLv3 or later

// mapping geda-symbols to actual devices
// "spice" section

module SPICE-VC-switch(1 2 3 4);
// check: are these the spice defaults?
 	parameter ron=1;
 	parameter roff=1e12;
 	parameter vt=0;
 	parameter vh=1;
 	paramset sss sw;
  	 .vt=vt;
 	 .vh=vh;
 	 .ron=ron;
 	 .roff=roff;
 	endparamset
 	sss #() s(1,2,3,4);
endmodule

// .. so it does not show up in netlist
hidemodule SPICE-VC-switch

simulator lang=acs
