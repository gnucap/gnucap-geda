v 20130925 2
C 40000 40000 0 0 0 title-B.sym
N 47300 46600 47300 47300 4
C 43500 47200 1 0 0 input-2.sym
{
T 42900 47400 5 10 1 0 0 0 1
net=INPUT:1
T 44200 47500 5 10 1 1 0 0 1
device=port
T 44000 47300 5 10 1 1 0 7 1
value=INPUT
}
C 53500 47200 1 0 0 output-2.sym
{
T 54400 47400 5 10 1 0 0 0 1
net=nout
T 53700 47500 5 10 1 1 0 0 1
device=port
T 54400 47300 5 10 1 1 0 1 1
value=OUTPUT
}
C 45500 47200 1 0 0 resistor-2.sym
{
T 45900 47550 5 10 0 0 0 0 1
device=RESISTOR
T 45700 47500 5 10 1 1 0 0 1
refdes=R1
T 45400 46900 5 10 1 0 0 0 1
value=4k
}
N 45500 47300 44900 47300 4
N 47300 45200 47300 45700 4
C 47200 45200 1 270 0 passive-1.sym
{
T 47500 44600 5 10 1 0 0 0 1
net=PASSIVE:1
T 47900 45000 5 10 0 0 270 0 1
device=none
T 47500 44500 5 10 1 1 0 1 1
value=PASSIVE
}
C 47500 45700 1 90 0 capacitor-1.sym
{
T 48700 46500 5 10 1 1 180 0 1
device=CAPACITOR
T 47900 46200 5 10 1 1 180 0 1
refdes=C1
T 46600 45900 5 10 0 0 90 0 1
symversion=0.1
T 47500 45700 5 10 1 0 0 0 1
value=1u
}
N 46400 47300 53500 47300 4
{
T 50600 47400 5 10 1 1 0 0 1
netname=teetop
}
