
simulator lang=spice

.options noinsensitive

.model cmosn nmos (                                level   = 49
+alpha0 = 6.27411e-08 beta0 = 11.59263
+version = 3.1            tnom    = 27             tox     = 4.1e-9
+xj      = 1e-7           nch     = 2.3549e17      vth0    = 0.367
+k1      = 0.592797       k2      = 2.518108e-3    k3      = 1e-3
+k3b     = 4.7942179      w0      = 1e-7           nlx     = 1.745125e-7
+dvt0w   = 0              dvt1w   = 0              dvt2w   = 0
+dvt0    = 1.3683195      dvt1    = 0.41           dvt2    = 0.0552615
+u0      = 263.5112775    ua      = -1.363381e-9   ub      = 2.253823e-18
+uc      = 4.833037e-11   vsat    = 1.017805e5     a0      = 1.9261289
+ags     = 0.4192338      b0      = -1.069507e-8   b1      = -1e-7
+keta    = -8.579587e-3   a1      = 2.789024e-4    a2      = 0.8916186
+rdsw    = 126.5291844    prwg    = 0.4957859      prwb    = -0.2
+wr      = 1              wint    = 0              lint    = 7.790316e-9
+xl      = -4e-8          xw      = 0              dwg     = -1.224589e-8
+dwb     = 1.579145e-8    voff    = -0.0895222     nfactor = 2.5
+cit     = 0              cdsc    = 2.4e-4         cdscd   = 0
+cdscb   = 0              eta0    = 2.95614e-3     etab    = 1.374596e-4
+dsub    = 0.014          pclm    = 0.73           pdiblc1 = 0.133
+pdiblc2 = 2.151668e-3    pdiblcb = -0.1           drout   = 0.6947618
+pscbe1  = 7.412661e10    pscbe2  = 1.812826e-7    pvag    = 9.540595e-3
+delta   = 0.01           rsh     = 5.9            mobmod  = 1
+prt     = 0              ute     = -1.5           kt1     = -0.11
+kt1l    = 0              kt2     = 0.022          ua1     = 4.31e-9
+ub1     = -7.61e-18      uc1     = -5.6e-11       at      = 3.3e4
+wl      = 0              wln     = 1              ww      = 0
+wwn     = 1              wwl     = 0              ll      = 0
+lln     = 1              lw      = 0              lwn     = 1
+lwl     = 0              capmod  = 2              xpart   = 0.5
+cgdo    = 8.71e-10       cgso    = 8.71e-10       cgbo    = 1e-12
+cj      = 9.67972e-4     pb      = 0.6966474      mj      = 0.3609772
+cjsw    = 2.443898e-10   pbsw    = 0.8082076      mjsw    = 0.1013742
+cjswg   = 3.3e-10        pbswg   = 0.8082076      mjswg   = 0.1013742
+cf      = 0              pvth0   = 7.226579e-4    prdsw   = -4.5298309
+)

.model cmosp pmos (                                level   = 49
+alpha0 = 6.27411e-08 beta0 = 11.59263
+version = 3.1            tnom    = 27             tox     = 4.1e-9
+xj      = 1e-7           nch     = 4.1589e17      vth0    = -0.4002789
+k1      = 0.5772615      k2      = 0.026742       k3      = 0
+k3b     = 14.2532769     w0      = 1e-6           nlx     = 9.883899e-8
+dvt0w   = 0              dvt1w   = 0              dvt2w   = 0
+dvt0    = 0.6718731      dvt1    = 0.3118588      dvt2    = 0.1
+u0      = 118.0541064    ua      = 1.626518e-9    ub      = 1.229265e-21
+uc      = -1e-10         vsat    = 2e5            a0      = 1.8109799
+ags     = 0.4096261      b0      = 7.705744e-7    b1      = 2.657048e-6
+keta    = 0.0212376      a1      = 0.5260122      a2      = 0.3207082
+rdsw    = 306.4304418    prwg    = 0.5            prwb    = 0.0612789
+wr      = 1              wint    = 0              lint    = 2.043723e-8
+xl      = -4e-8          xw      = 0              dwg     = -4.602158e-8
+dwb     = 8.005928e-9    voff    = -0.0992452     nfactor = 2
+cit     = 0              cdsc    = 2.4e-4         cdscd   = 0
+cdscb   = 0              eta0    = 0.0331989      etab    = -0.0375363
+dsub    = 0.7            pclm    = 1.5            pdiblc1 = 2.7e-4
+pdiblc2 = 0.0165863      pdiblcb = -1e-3          drout   = 1.64e-4
+pscbe1  = 7.72e9         pscbe2  = 2.228426e-9    pvag    = 5.1166248
+delta   = 0.01           rsh     = 6.7            mobmod  = 1
+prt     = 0              ute     = -1.5           kt1     = -0.11
+kt1l    = 0              kt2     = 0.022          ua1     = 4.31e-9
+ub1     = -7.61e-18      uc1     = -5.6e-11       at      = 3.3e4
+wl      = 0              wln     = 1              ww      = 0
+wwn     = 1              wwl     = 0              ll      = 0
+lln     = 1              lw      = 0              lwn     = 1
+lwl     = 0              capmod  = 2              xpart   = 0.5
+cgdo    = 6.92e-10       cgso    = 6.92e-10       cgbo    = 1e-12
+cj      = 1.173089e-3    pb      = 0.8524959      mj      = 0.415401
+cjsw    = 2.217367e-10   pbsw    = 0.594          mjsw    = 0.26
+cjswg   = 4.22e-10       pbswg   = 0.59      mjswg   = 0.26
+cf      = 0              pvth0   = 1.426e-3    prdsw   = 0.989
+ )

.simulator lang=verilog
