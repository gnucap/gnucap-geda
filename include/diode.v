simulator language=verilog
// This File is part of gnucap-geda
// (C) 2015 Felix Salfelder
// GPLv3 or later

// mapping geda-symbols to actual devices
// "diode" section

// what are useful defaults here?
module DIODE(1 2);
parameter n=1;
paramset  ddd  d;
.is= 10.f;
.rs= 0.;
.n={n};
.tt= 0.;
.cjo= 1.p;
.vj= 1.;
.m= 0.5;
.eg= 1.11;
.xti= 3.;
.kf= 0.;
.af= 1.;
.fc= 0.5;
.bv= 50;
.ibv= 0.001;
endparamset
ddd d1(1,2);
endmodule

hidemodule DIODE

module LEDseries(1 2);
// incomplete
endmodule

simulator lang=acs
