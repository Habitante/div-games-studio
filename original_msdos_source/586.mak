#컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
#       Fichero de compilaci줻 de DIV
#컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴

d.exe: div.obj divpaint.obj divbasic.obj divpalet.obj divkeybo.obj divsprit.obj
d.exe: divvideo.obj divmouse.obj divsetup.obj divwindo.obj divlengu.obj
d.exe: divbrow.obj divgama.obj divhandl.obj divforma.obj divasm.obj divcdrom.obj
d.exe: divbin.obj grabador.obj fpgfile.obj divfont.obj ifs.obj divpcm.obj
d.exe: divedit.obj divdsktp.obj divc.obj divhelp.obj diveffec.obj divinsta.obj
d.exe: divdll1.obj divdll2.obj divcolor.obj divcalc.obj divfpg.obj cdrom.obj
d.exe: divmix.obj divspr.obj divmap3d.obj divsb.obj vesa.obj det_vesa.obj
d.exe: divsound.obj divmixer.obj timer.obj divtimer.obj

  wlink @d.lnk

div.obj: div.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r div.cpp

divkeybo.obj: divkeybo.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divkeybo.cpp

divpaint.obj: divpaint.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divpaint.cpp

divbasic.obj: divbasic.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divbasic.cpp

divpalet.obj: divpalet.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divpalet.cpp

divsprit.obj: divsprit.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divsprit.cpp

divvideo.obj: divvideo.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divvideo.cpp

divmouse.obj: divmouse.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divmouse.cpp

divsetup.obj: divsetup.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divsetup.cpp

divwindo.obj: divwindo.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divwindo.cpp

divlengu.obj: divlengu.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divlengu.cpp

divbrow.obj: divbrow.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divbrow.cpp

divgama.obj: divgama.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divgama.cpp

divhandl.obj: divhandl.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divhandl.cpp

divforma.obj: divforma.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divforma.cpp

#divasm.obj: divasm.cpp
#  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divasm.cpp

divasm.obj: divasm.asm
  wasm divasm.asm

divcdrom.obj: divcdrom.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divcdrom.cpp

cdrom.obj: cdrom.cpp  cdrom.h
  wcc386 /oneatx /wx /fp5 /mf /bt=dos /5r cdrom.cpp

divfpg.obj: divfpg.cpp  fpgfile.hpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divfpg.cpp

divbin.obj: divbin.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divbin.cpp

grabador.obj: grabador.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r grabador.cpp

fpgfile.obj: fpgfile.cpp  fpgfile.hpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r fpgfile.cpp

divedit.obj: divedit.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divedit.cpp

divfont.obj: divfont.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divfont.cpp

ifs.obj: ifs.cpp
  wcc386 /oneatx /wx /fp5 /mf /bt=dos /5r ifs.cpp

divpcm.obj: divpcm.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divpcm.cpp

divdsktp.obj: divdsktp.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divdsktp.cpp

divc.obj: divc.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divc.cpp

divhelp.obj: divhelp.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divhelp.cpp

diveffec.obj: diveffec.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r diveffec.cpp

divinsta.obj: divinsta.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divinsta.cpp

divdll1.obj: divdll1.c
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divdll1.c

divdll2.obj: divdll2.c
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divdll2.c

divcolor.obj: divcolor.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divcolor.cpp

divcalc.obj: divcalc.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divcalc.cpp

divmix.obj: divmix.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divmix.cpp

divspr.obj: divspr.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divspr.cpp

divmap3d.obj: divmap3d.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divmap3d.cpp

divsb.obj: divsb.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divsb.cpp

vesa.obj: vesa.asm
  a vesa.asm /ml /m2 /w2 /z /zn

det_vesa.obj: det_vesa.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r det_vesa.cpp
# wcc386 det_vesa.cpp /mf /oneatx /w3 /d__WC32__

divsound.obj: divsound.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divsound.cpp

divmixer.obj: divmixer.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divmixer.cpp

timer.obj: timer.asm
        a /m /ml timer.asm

divtimer.obj: divtimer.cpp
  wcc386 /oneatx /wx /zp4 /fp5 /mf /bt=dos /5r divtimer.cpp

