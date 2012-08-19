'
v 20110115 2
C 46400 44400 1 0 0 resistor-1.sym
{
T 46400 44400 5 10 0 1 0 0 1
refdes=Re1
T 46400 44400 5 10 0 1 0 0 1
value=150
}
C 47700 44400 1 0 0 resistor-1.sym
{
T 47700 44400 5 10 0 1 0 0 1
refdes=Re2
T 47700 44400 5 10 0 1 0 0 1
value=150
}
C 48800 46900 1 90 0 resistor-1.sym
{
T 48800 46900 5 10 0 1 0 0 1
refdes=Rc
T 48800 46900 5 10 0 1 0 0 1
value=20k
}
C 47700 44200 1 90 1 current-1.sym
{
T 47700 44200 5 10 0 1 0 0 1
refdes=Ic
T 47700 44200 5 10 0 1 0 0 1
value=0.5mA
}
C 45700 45400 1 0 0 BC547-1.sym
{
T 45700 45400 5 10 0 1 0 0 1
refdes=Q1
T 45700 45400 5 10 0 1 0 0 1
m=NA( NA)
}
C 49300 45400 1 0 1 BC547-1.sym
{
T 49300 45400 5 10 0 1 0 0 1
refdes=Q2
T 49300 45400 5 10 0 1 0 0 1
m=NA( NA)
}
C 47300 47800 1 0 0 vcc-1.sym
{
T 47300 47800 5 10 0 1 0 0 1
refdes=!_vcc-1.sym1302993932
T 47300 47800 5 10 0 1 0 0 1
value=10
}
C 47200 42700 1 0 0 vcc-minus-1.sym
{
T 47200 42700 5 10 0 1 0 0 1
refdes=!_vcc-minus-1.sym312780053
T 47200 42700 5 10 0 1 0 0 1
value=-10
}
N 46300 47800 46300 46400 4
N 46300 45400 46300 44500 4
N 46300 44500 47300 44500 5
N 46300 44500 46400 44500 4
N 47300 44500 47700 44500 4
N 48600 44500 47300 44500 5
N 48600 44500 48700 44500 4
N 48700 44500 48700 45400 4
N 47500 44200 47500 44500 4
N 48700 46900 48700 46400 4
C 44900 45800 1 0 0 input-1.sym
{
T 44900 45800 5 10 0 1 0 0 1
refdes=!_input-1.sym1880336771
T 44900 45800 5 10 0 1 0 0 1
pinlabel=Vi
}
C 49200 45600 1 0 0 gnd-1.sym
C 48700 46500 1 0 0 output-1.sym
{
T 48700 46500 5 10 0 1 0 0 1
refdes=!_output-1.sym567755454
T 48700 46500 5 10 0 1 0 0 1
pinlabel=Vo
}
N 46300 47800 48700 47800 4
