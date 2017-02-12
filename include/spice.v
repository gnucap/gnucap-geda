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

simulator lang=spice
.options noinsensitive
.subckt SPICE-vcvs(1 2 3 4);
.parameter value=1
.parameter ac=1
E1 1 2 3 4 dc={value} ac={ac}
.ends

* are the defaults correct?
.subckt vsin 1 2
.parameter amplitude=1
.parameter frequency=5e6
.parameter ac=1
V1 1 2 sin amplitude=amplitude frequency=frequency ac=ac
.ends

.simulator lang=acs
hidemodule SPICE-vcvs
hidemodule vsin
