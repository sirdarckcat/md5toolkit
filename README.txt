
MD5 Toolkit
===========

This package contains code to produce MD5 collisions.  The core of the
toolkit are the two programs, 'block1' and 'block2'.

block1 produces a pair M_1 and M'_1 of 512-bit blocks that satisfy the
Wang-differential.  It also produces the MD5 chaining value for M_1.

block2 needs only the M_1 chaining value (the M'_1 chaining value can
be determined from it), and it then outputs a pair M_2 and M'_2.  

At the end, we have that M_1 || M_2  collides with  M'_1 || M'_2
under MD5.


Auxiliary Programs
==================

Both block1 and block2 output their results in ASCII.  The program
'makeblocks' converts the output from these programs into two binary
files so that MD5 can be run on them ('md5sum' is the typical program
used for Linux installations).

A shell script 'makebins.sh' is also provided in the makeblocks/
directory, which does all the steps needed to produce a sample collision.

makebins.sh first runs block1, saving its output in a file 'block1.out'.
Then it runs 'block2' and saves its output in 'block2.out'.  Appending
these outputs into a single file, it hands them to 'makeblocks' along
with output filenames b1.bin and b2.bin.

b1.bin and b2.bin are distinct files, each 1024-bits long, which collide
under MD5.  md5sum is run on each of these files and the user may verify
the digests are the same.


Building the Code
=================

sh build.sh

builds all the code.



Efficiency
==========

The speed of the code, as usual, depends on several factors: the hardware
used, the quality of the compiler, and how well the code has been tuned.
(This under the understanding that the basic algorithm remains the same.)

We have tested the code on a Pentium4 (Xeon) with g++ 3.4.4.  (Other
architectures can be specified in the build.sh file.)  The code
was also tested with the Intel icc compiler, and it runs significantly
faster, but this compiler is not as widely available, so we've stuck
to g++ for our Makefiles.

The code has not yet been tuned anywhere near its potential.  Currently,
in the environment cited above, the block1 code runs typically from 8 to
50 minutes before producing a collision.  The block2 code runs about 3 seconds
to 5 minutes, typically.  Running 100 trials of each program, we've 
measured 16 mins as the average time for block1 and 2 mins for the average
time for block2 (with large variations, however).

Therefore we expect to see collisions in under 20 mins, though sometimes
much slower or much faster.

Tuning the code makes things run much faster.  Using the faster code from
http://www.stachliu.com/collisions.html
for the block1 algorithm allows us to produce collisions in about 11 minutes.
Efforts are underway to tune the code for both blocks; preliminary results
show we can expect collisions in about 5 mins.

Wang's original code ran in about an hour on an IBM P690 supercomputer.
The Stach-Liu code cited above runs in about 45 minutes on a Pentium4,
however their code sometimes produces block1 pairs that have no block2
solutions (our code never does this).


Applications
============

We designed the toolkit so that it would be easy to generates various
flavors of collisions.  For example, many collisions where the first
blocks are the same between a given pair, but the second blocks vary.
(We can get lots of these fast because our block2 code is quite fast
even before we have tuned it much.)

Also, we can generate collisions for arbitrary IVs.  This is useful for
attacks like the Daum-Lucks attack (http://www.cits.rub.de/MD5Collisions).
As an application of their idea we produced two binaries that have the
same MD5 digest but where binary1 prints "hello world" and binary2 
prints "I am erasing your hard disk" (even though are program really doesn't
erase your hard disk).  This is done by having the program check to see
whether msg1 is in a given array, or msg2 is (where msg1 and msg2 are 
1024-bit values that collide under MD5 with the chaining value resulting
from hashing the binary to the point where the msg1 or msg2 value occurs).

