README.txt
Last updated:  1/04/2006

This file is intended to supplement the code found in block1.c.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Building the Executable
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

To build the binary, just type 'make' at the command line, or use the command

        gcc -O3 -march=pentium4 block1.c -o block1

where the '-march=' flag is changed to the appropriate value.  On the
Pentium 4, use of this flag makes the code roughly twice as fast compared
to a binary compiled without specifying the architecture.  You can also
use the build.sh script supplied to build the entire toolkit.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Usage
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

The code can either use the default IV for MD5, or it can take the
IV as a paramter.  In the former case, the code is invoked by

        ./block1

and in the latter case the code is invoked by

        ./block 1 <IV>

where IV is any hex value of length 32.  As an example, one might invoke the
code as follows:

        ./block1 d41d8cd98f00b204e9800998ecf8427e


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Output
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

The output to the program consists of three parts: the output of the
MD5 compression function on the first of two messages, M and M', and
then the two 512-bit messages M and M' themselves,
represented as a 16-tuple of 32-bit hex values.

As an example run, we obtained the following output:

    Chaining value for M:
    ef5f6cf37991e593628c40e6794a54b9
    M = {   87e85516, 9f820a2d, 1d2c1ea0, 891cc06e, 
            b50347ae, 364a887c, 3ada98ae, 62468e31, 
            05352d45, 21333bfe, 8c9ef6b7, 269a3354, 
            6043a0c7, 3f98a2f4, ab400728, 3a995dcd }

    M' = {  87e85516, 9f820a2d, 1d2c1ea0, 891cc06e, 
            350347ae, 364a887c, 3ada98ae, 62468e31, 
            05352d45, 21333bfe, 8c9ef6b7, 269ab354, 
            6043a0c7, 3f98a2f4, 2b400728, 3a995dcd }

This output is a pair of first-blocks of what will be a pair of two-block
messages that collide under MD5.  The differential is the Wang-differential,
but several more conditions were specified to speed up the algorithm as
described in the accompanying paper (and see below).

The chaining value for M is all that is needed in order to produce a pair
of second-blocks to complete the collision-pair.  The chaining value above
is given to a separate program (aptly named 'block2') to accomplish this.
When M from this program is prepended to M from the block2 program we get
a two-block message X.  When M' from this program is prepended to M' from
the block2 program, we get a distinct two-block message Y.  X and Y will
collide under MD5.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
md5cond_1.txt
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

This file contains the list of conditions on the step values computed during
the computation of the MD5 compression function.  For 'normal' use of the
code, no modifications of md5cond_1.txt are necessary.  That is, the file
contains an encoding of all the conditions present in the associated
paper.  One may wish to change md5cond_1.txt only to experiment with
new conditions or to examine the effect on code running-time when various
conditions are manipulated, created, or removed.

Format:
Each line of the file encodes one condition and is composed of five
space-delimited numbers.  The first number denotes the step number of the
condition, with acceptable values of 0-63 (68-71 are used for the conditions
on the chaining values).  The second number denotes the index of the bit of
the condition (values 0-31).  The third value denotes either the value of
that bit (-2 means that bit should be zero, and -1 means that bit should be
one) or the index of the step value that it should be compared to.  The
fourth and fifth values are used only when the condition refers to another
step value and they represent the bit index and additive constant to that
value, respectively.

Examples:
3 5 -2 0 0
Bit 5 on step value 3 should be 0.

27 31 -1 0 0
Bit 31 on step value 27 should be 1.

15 31 14 31 0
Bit 31 on step value 15 should be the same as bit 31 on step value 14.

63 31 61 31 1
Bit 31 on step value 63 should be the opposite of bit 31 on step value 61.
