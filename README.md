SuperPC
=======

An emulator of the IBM PC 5150, and its descendants and clones.

License
=======
attotime.h, attotime.cpp, eminline.h, and emucore.h are all under the BSD 3 Clause License. These files are from MAME.

Compiling and running
=====================

To compile SuperPC you will need to install g++, yasm and SDL. After you've done that run:

make; ./superpc mda.cfg > log.txt

You should see a black window on the screen and log.txt should contain everything the emulator has been doing.

This emulator uses the same BIOS pack as 86Box.

Todo
====

ISA DMA
-------

I don't quite understand how transfers are started.

MDA
---

Only updates every frame. Might want to change that eventually.

CGA
---

It's got basic text and graphics capabilities.

8088
----

Only accurate on an opcode-level.

80286
-----

VERY rudimentary.

PIT
---

Not complete, but the basics are there.

PPI
---

Much to be done here.

PIC
---

It's sorta there, but still incomplete.
