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
hidemodule CAPACITOR

module POLARIZED_CAPACITOR(p n);
// nothing yet
endmodule
hidemodule POLARIZED_CAPACITOR

module RESISTOR(1 2);
	parameter value=1;
	resistor #(.r(value)) r(1,2);
endmodule
hidemodule RESISTOR

module BATTERY(n p);
	parameter value=1;
	vsource #(.dc(value)) v(n,p);
endmodule
hidemodule BATTERY

module VOLTAGE_SOURCE(1 2);
	parameter value=1;
	vsource #(.dc(value)) v(1 2);
endmodule
hidemodule VOLTAGE_SOURCE

module CURRENT_SOURCE(1 2);
	parameter value=1;
	isource #(.dc(value)) i(1 2);
endmodule
hidemodule CURRENT_SOURCE

module BC547(1 2 3 4);
// nothing yet
endmodule
hidemodule BC557

module TRANSFORMER(1 2 3 4 5 6 7);
// not yet
endmodule
hidemodule TRANSFORMER

module NMOS_TRANSISTOR(D G S B);
paramset cmosn_local nmos;
.level=1; .kp=41.5964u; .vto=0.8; .gamma=0.863074; .phi=0.6; .lambda=.01;
.tox=41.8n; .nsub=15.3142E+15; .nss=1.E+12; .xj=400.n; .uo=503.521; .tpg=1;
.fc=0.5; .pb=0.7; .cj=384.4u; .mj=0.4884; .cjsw=527.2p; .mjsw=0.3002; .js=0.;
.rsh=0.; .cbd=0.; .cbs=0.; .cgso=218.971p; .cgdo=218.971p; .cgbo=0.;
.ld=265.073n;
endparamset
parameter l
parameter w
cmosn_local #(.l(l), .w(w)) m(D,G,S,B);
endmodule
hidemodule NMOS_TRANSISTOR

module PMOS_TRANSISTOR(D G S B);
paramset local_cmosp pmos;
.level=1; .vto=-0.8; .kp= 41.5964u; .gamma= 0.863074; .phi= 0.6; .rd= 0.;
.rs= 0.; .cbd= 0.; .cbs= 0.; .is= 0; .pb= 0.7; .cgso= 218.971p;
.cgdo=218.971p; .cgbo= 0.; .rsh= 0.; .cj= 384.4u; .mj= 0.4884; .cjsw= 527.2p;
.mjsw= 0.3002; .js= 0.; .tox= 41.8n; .nsub= 15.3142E+15; .nss= 1.E+12; .tpg=1;
.xj= 400.n; .ld= 265.073n; .uo= 503.521; .kf= 0.; .af= 1.; .fc= 0.5;
.lambda=.01;
endparamset
parameter l
parameter w
local_cmosp #(.l(l), .w(w)) m(D,G,S,B);
endmodule
hidemodule PMOS_TRANSISTOR

module NPN_TRANSISTOR(C B E);
// later..
endmodule
hidemodule NPN_TRANSISTOR

module LED(CATHODE, ANODE);
// nothing
endmodule
hidemodule LED

simulator lang=acs
