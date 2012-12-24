* a spice module, testing spice-sdb style "file" attribute

.subckt foo A B
.model foores res (TC1=1.9 TC2=2.43u DEFW=1 RSH=1)
Rfoo A B foores L=1k
.ends
