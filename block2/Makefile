MARCH=pentium4
CPPFLAGS= -O3 -march=$(MARCH)

default: block2

md5.o: md5.cpp md5.h

block2.o: block2.cpp

block2: md5.cpp block2.cpp md5.o block2.o
	g++ block2.o md5.o -o block2

clean:
	rm -f *.o block2
