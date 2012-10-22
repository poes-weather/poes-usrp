libfec-3.0.1 By Phil Karn
http://www.ka9q.net/code/fec/
Download: http://www.ka9q.net/code/fec/fec-3.0.1.tar.bz2
untar the standard fec-3.0.1 package

===========
x64 systems
===========

Download and apply the patch
http://lodge.glasgownet.com/bitsnbobs/kg_fec-3.0.1.patch
$ patch < kg_fec-3.0.1.patch

Check that the below changes were made

===========
makefile.in
===========
Replace (line 21)
  CFLAGS=@CFLAGS@ -I. -Wall @ARCH_OPTION@
by 
  CFLAGS=@CFLAGS@ -I. -Wall @ARCH_OPTION@ -fPIC

Replace (line 103)
  gcc -shared -Xlinker -soname=$@ -o $@ -Wl,-whole-archive $^ -Wl,-no-whole-archive -lc
by
  gcc -shared -Xlinker -soname=$@ -o $@ -Wl,-whole-archive $^ -Wl,-no-whole-archive -lc -lm

=====
fec.h
=====
libfec has a compilation error (" fec.h:267: Error: bad register name `%dil' ")

Replace (line 267)
  __asm__ __volatile__ ("test %1,%1;setpo %0" : "=g"(x) : "r" (x));
by
  __asm__ __volatile__ ("test %1,%1;setpo %0" : "=q"(x) : "q" (x));

===============
build & install
===============
$ ./configure
$ make test
$ sudo make install

===================
Link with poes-usrp
===================
Edit: POES-DECODER.pro

link with libfec: Line 185-186 (remove #)

    DEFINES += HAVE_LIBFEC
    LIBS += -L/usr/local/lib -lfec

Rebuild and Run. If you get error Permission denied see 'SELinux Policy' below

==============
SELinux Policy
==============
sudo setsebool -P allow_execstack 1

