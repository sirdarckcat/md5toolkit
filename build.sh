#!/bin/bash

# Change this line to appropriate architecture
export MARCH=pentium4

if [ "$#" -eq 0 ]
then
    echo '******Building block1 code******'
    cd block1
    make -e

    echo '******Building block2 code******'
    cd ../block2
    make -e


    echo '******Building makeblocks******'
    cd ../makeblocks
    make -e
    cp ../block1/block1 .
    cp ../block2/block2 .

    if [ -f 'md5cond_1.txt' ] 
    then
        echo '' > /dev/null
    else
        ln -s ../block1/md5cond_1.txt md5cond_1.txt
    fi
    if [ -f 'md5cond2.txt' ] 
    then
        echo '' > /dev/null
    else
        ln -s ../block2/md5cond2.txt md5cond2.txt
    fi


    exit 0
fi

case "$1" in 
clean)
    echo '******Make clean******'
    cd block1
    make clean
    cd ../block2
    make clean
    cd ../makeblocks
    make clean
    ;;
*)
    echo 'Unrecognized parameter'
    ;;
esac
