
simulator language=verilog
// This File is part of gnucap-geda
// (C) 2012 Felix Salfelder
// GPLv3 or later

// mapping geda-symbols to actual devices
// "switch" section

// module SPST_doesnotwork(1,2);
// 	parameter value=1.;
// 	admittance #(.g((1.-value)*1e6)) a(1,2);
// endmodule

simulator language=spice
.options noinsensitive

.subckt SPST 1 2
.parameter value=0.
.parameter y={(value)*1e6}
Y1 1 2 ac {y} dc {y} tran {y} op {y}
.ends

.hidemodule SPST

.simulator language=verilog
