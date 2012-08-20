'
v 20081231 1 
N 47500 45600 47500 46900 4
N 48200 46900 48400 46900 4
N 48100 47500 48100 47400 4
T 51800 40900 9 16 1 0 0 0 1 
Inverter Gate 
T 50100 40400 9 10 1 0 0 0 1 
inverter.sch 
T 50500 40100 9 10 1 0 0 0 1 
1 
T 52200 40100 9 10 1 0 0 0 1 
1 
T 54200 40400 9 10 1 0 0 0 1 
1 
T 54300 40100 9 10 1 0 0 0 1 
Facundo J Ferrer 
T 43300 48400 5 10 0 1 0 0 1 
device=spice-subcircuit-LL 
T 43300 48500 5 10 1 1 0 0 1 
refdes=A1 
T 43300 48200 5 10 1 1 0 0 1 
model-name=cmos_inverter 
} 
C 48900 45900 1 0 0 spice-subcircuit-IO-1.sym
{
T 48900 45900 5 10 0 1 0 0 1
device=spice-IO
T 48900 45900 5 10 0 1 0 0 1
refdes=P1
}
C 47300 46000 1 0 0 spice-subcircuit-IO-1.sym
{
T 47300 46000 5 10 0 1 0 0 1
device=spice-IO
T 47300 46000 5 10 0 1 0 0 1
refdes=P4
}
C 47900 47200 1 0 0 spice-subcircuit-IO-1.sym
{
T 47900 47200 5 10 0 1 0 0 1
device=spice-IO
T 47900 47200 5 10 0 1 0 0 1
refdes=P2
}
C 47900 44400 1 0 0 spice-subcircuit-IO-1.sym
{
T 47900 44400 5 10 0 1 0 0 1
device=spice-IO
T 47900 44400 5 10 0 1 0 0 1
refdes=P3
}
T 43400 47600 5 10 0 1 0 0 1 
device=model 
T 43400 47500 5 10 1 1 0 0 1 
refdes=A2 
T 44600 47200 5 10 1 1 0 0 1 
model-name=nmos4 
T 43800 47000 5 10 1 1 0 0 1 
file=../model/nmos4.model 
} 
T 43400 46400 5 10 0 1 0 0 1 
device=model 
T 43400 46300 5 10 1 1 0 0 1 
refdes=A3 
T 44600 46000 5 10 1 1 0 0 1 
model-name=pmos4 
T 43800 45800 5 10 1 1 0 0 1 
file=../model/pmos4.model 
} 
N 48400 46900 48400 47400 4
N 48400 47400 48100 47400 4
N 48400 44700 48100 44700 4
N 48100 46200 49100 46200 4
C 47500 45100 1 0 0 asic-nmos-1.sym
{
T 47500 45100 5 10 0 1 0 0 1
device=NMOS_TRANSISTOR
T 47500 45100 5 10 0 1 0 0 1
refdes=M2
T 47500 45100 5 10 0 1 0 0 1
l=3u
T 47500 45100 5 10 0 1 0 0 1
model-name=nmos4
T 47500 45100 5 10 0 1 0 0 1
w=1u
}
C 47500 46400 1 0 0 asic-pmos-1.sym
{
T 47500 46400 5 10 0 1 0 0 1
device=PMOS_TRANSISTOR
T 47500 46400 5 10 0 1 0 0 1
refdes=M1
T 47500 46400 5 10 0 1 0 0 1
l=1u
T 47500 46400 5 10 0 1 0 0 1
model-name=pmos4
T 47500 46400 5 10 0 1 0 0 1
w=10u
}
N 48100 46100 48100 46400 4
N 48400 44700 48400 45600 4
N 48400 45600 48200 45600 4
N 48100 44700 48100 45100 4
