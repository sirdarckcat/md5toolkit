#!/bin/bash

echo 'This script builds a pair of two-block messages that collide under MD5.'
echo

K=1

#while [ $K -le 20 ]
#do
    echo 'Running block1 code.'
    time ./block1 > block1.out
    echo 'Done with first block.'

    sed -e 's/[a-zA-Z0-9 \t]*[:,}]/M/g' block1.out | grep "^[^M]" > chain.out
    mv CV.txt CV.txt.bak
    mv chain.out CV.txt
    time ./block2 > block2.out
    echo 'Done with second block.'

    cat block1.out block2.out > blocks.out

    echo 'Converting to binary files...'
    ./makeblocks blocks.out b1.bin b2.bin

    echo 'And here are the MD5 digests:'
    md5sum b1.bin b2.bin

    K=$((K+1))
#done
