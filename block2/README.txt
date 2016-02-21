block2
======

This code produces the second block of an MD5 collision given the chaining
value from a first block message.  Using the chaining value from M output
by the block1 program, we are able to calculate the chaining value for M1'. 
The chaining values are specified in the file 'CV.txt'.

Compilation instructions
========================
	g++ -O3 -c md5.cpp
	g++ -O3 -c block2.cpp
	g++ block2.o md5.o -o Block2.out
	
At one point we noted that compiling the Block2 code with the Intel compiler
produced a 30% performance increase.  People having access to the Intel 
compiler are encouraged to use it.

icc -fast -Ob2 md5.cpp block2.cpp
The binary will be placed in a.out



Sufficient Conditions
======================

The sufficient conditions for producing a second block message are
contained in md5cond2.txt.  Conditions reflect the updated conditions
released by Jun Yajima and Takeshi Shemoyama.  These conditions may
be modified by researchers when the list of sufficient conditions
is changed.  It should also be noted that conditions are "manually"
satisfied for portions of the code rather than looking through the list
of condtions for efficiency reasons.  In md5cond2.txt each condition is
represented by a three tuple (x,y,z).  x is the step value the condition
is placed on and has value [0-63].  y is the bit within the step value
that the condition is placed on, and has values [1-32].  z is the type
of condition having value [0-6]

0	-bit y of stepvalue x should be 0
1	-bit y of stepvalue x should be 1
2	-bit y of stepvalue x should be equal to bit y of stepvalue x-1
3	-bit y of stepvalue x should be equal to bit y of stepvalue x-2
4	-bit y of stepvalue x should be not equal to bit y of stepvalue x-1
5	-bit y of stepvalue x should be not equal to bit y of stepvalue x-2



Chaining Values
===============
CV.txt may have arbitrarily many chaining values.

First-block chaining values are used as the IV's for the second block.
Each set of chainging values is specified on a single line as a 128 bit value.


Executing block2 code
=====================

block2
block2 [num cv]
block2 [num cv] [num coll]

where:
	[num cv] is the number of chaining value sets to be read from CV.txt
	
	[num coll] is the number of collisions to be produced for each 
	chainging variable.
	
	When [num coll] is specified [num cv] must also be specified.
	When either [num coll] or both parameters are not specified they
	default to 1.


Output Format
=============

The output format is the same for block2 as it is for block1: the chaining
value for M is output first, then M and M' are output.  Here they are 
two 512-bit values intended as the second blocks corresponding to the
pair output from the block1 code.

The chaining value shown is NOT the same as the MD5 output value when
run on either colliding pair because our calculations were not done with
the length appended.  However, since both messages are two blocks long,
appending the length will not upset the collision.



Debugging
=========

Optional output has been left in the code for the purpose of dubugging.  This
output can be turned on by setting the DEBUG constant to 1 rather than 0.
