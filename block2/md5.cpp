#include <iostream>
using namespace std;
typedef unsigned int u_int;
void md5(u_int M[], u_int out[]);
void resetH();
u_int getY(int i);
void md5Step16(u_int M[],u_int vals[64]);
u_int getZ(int i);
u_int getS(int i);
u_int getB();
u_int F(u_int b, u_int c, u_int d, u_int m);
void md5Step(u_int M[],u_int out[], int j);
void md5Step(u_int M[],u_int out[], u_int Mprime[],u_int out1[],int j);
void md5Step16(u_int M[],u_int vals[64],u_int Mprime[],u_int vals1[64]);
void md5Step20(u_int M[],u_int vals[68],u_int Mprime[],u_int vals1[68]);

const u_int y [] = {  //addition constant
   0xd76aa478,   0xe8c7b756,   0x242070db,   0xc1bdceee,
   0xf57c0faf,   0x4787c62a,   0xa8304613,   0xfd469501,
   0x698098d8,   0x8b44f7af,   0xffff5bb1,   0x895cd7be,
   0x6b901122,   0xfd987193,   0xa679438e,   0x49b40821,

 /* Round 2 */
   0xf61e2562,   0xc040b340,   0x265e5a51,   0xe9b6c7aa,
   0xd62f105d,   0x2441453,    0xd8a1e681,   0xe7d3fbc8,
   0x21e1cde6,   0xc33707d6,   0xf4d50d87,   0x455a14ed,
   0xa9e3e905,   0xfcefa3f8,   0x676f02d9,   0x8d2a4c8a,

  /* Round 3 */
   0xfffa3942,   0x8771f681,   0x6d9d6122,   0xfde5380c,
   0xa4beea44,   0x4bdecfa9,   0xf6bb4b60,   0xbebfbc70,
   0x289b7ec6,   0xeaa127fa,   0xd4ef3085,    0x4881d05,
   0xd9d4d039,   0xe6db99e5,   0x1fa27cf8,   0xc4ac5665,

  /* Round 4 */
   0xf4292244,   0x432aff97,   0xab9423a7,   0xfc93a039,
   0x655b59c3,   0x8f0ccc92,   0xffeff47d,   0x85845dd1,
   0x6fa87e4f,   0xfe2ce6e0,   0xa3014314,   0x4e0811a1,
   0xf7537e82,   0xbd3af235,   0x2ad7d2bb,   0xeb86d391
};
const u_int z [] = {     // index
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
	1,6,11,0,5,10,15,4,9,14,3,8,13,2,7,12,
    5,8,11,14,1,4,7,10,13,0,3,6,9,12,15,2,
    0,7,14,5,12,3,10,1,8,15,6,13,4,11,2,9
};

const u_int s [] = {   //shift amount
	7,12,17,22,7,12,17,22,7,12,17,22,7,12,17,22,
    5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
    4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,
    6,10,15,21,6,10,15,21,6,10,15,21,6,10,15,21
   };


// returns constant for step i
u_int getY(int i)
{ 
	return y[i];
}

// returns word used in step i
u_int getZ(int i)
{ 
	return z[i];
}

// returns shift amount for step i
u_int getS(int i)
{ 
	return s[i];
}


//calculates the md5 for the message M
void md5(u_int M[],u_int vals[64])
{
	u_int a=vals[0];
	u_int b=vals[3];
	u_int c=vals[2];
	u_int d=vals[1];
	u_int t;
	u_int t1;
	for (int j=0; j<16;j++)
	{
		t=a+((b&c)|((~b)&d))+M[z[j]]+y[j];
		u_int temp = d;
		d=c;
		c=b;
		a=temp;
		t1 = t>>(32-s[j]);
		b=b+((t<<s[j])+t1);
		vals[j+4]=b;
	}
	for (int j=16; j<32;j++)
	{
		t=a+((b&d)|(c&~d))+M[z[j]]+y[j];
		u_int temp = d;
		d=c;
		c=b;
		a=temp;
		t1 = t>>(32-s[j]);
		b=b+((t<<s[j])+t1);
		vals[j+4]=b;
	}
	for (int j=32; j<48;j++)
	{
		t=a+(b^c^d)+M[z[j]]+y[j];
		u_int temp = d;
		d=c;
		c=b;
		a=temp;
		t1 = t>>(32-s[j]);
		b=b+((t<<s[j])+t1);
		vals[j+4]=b;
	}
	for (int j=48; j<64;j++)
	{
		t=a+(c^(b|~d))+M[z[j]]+y[j];
		u_int temp = d;
		d=c;
		c=b;
		a=temp;
		t1 = t>>(32-s[j]);
		b=b+((t<<s[j])+t1);
		vals[j+4]=b;
	}
}


//calculates the first 20 steps of the md5 for message M and Mprime
void md5Step20(u_int M[],u_int vals[68],u_int Mprime[],u_int vals1[68])
{
	u_int a=vals[0];
	u_int b=vals[3];
	u_int c=vals[2];
	u_int d=vals[1];
	u_int t;
	u_int t1;
	for (int j=0; j<16;j++)
	{
		t=a+((b&c)|((~b)&d))+M[z[j]]+y[j];
		u_int temp = d;
		d=c;
		c=b;
		a=temp;
		t1 = t>>(32-s[j]);
		b=b+((t<<s[j])+t1);
		vals[j+4]=b;
	}
	for (int j=16; j<21;j++)
	{
		t=a+((b&d)|(c&~d))+M[z[j]]+y[j];
		u_int temp = d;
		d=c;
		c=b;
		a=temp;
		t1 = t>>(32-s[j]);
		b=b+((t<<s[j])+t1);
		vals[j+4]=b;
	}
	a=vals1[0];
	b=vals1[3];
	c=vals1[2];
	d=vals1[1];
	t;
	t1;
	for (int j=0; j<16;j++)
	{
		t=a+((b&c)|((~b)&d))+Mprime[z[j]]+y[j];
		u_int temp = d;
		d=c;
		c=b;
		a=temp;
		t1 = t>>(32-s[j]);
		b=b+((t<<s[j])+t1);
		vals1[j+4]=b;
	}
	for (int j=16; j<21;j++)
	{
		t=a+((b&d)|(c&~d))+Mprime[z[j]]+y[j];
		u_int temp = d;
		d=c;
		c=b;
		a=temp;
		t1 = t>>(32-s[j]);
		b=b+((t<<s[j])+t1);
		vals1[j+4]=b;
	}
}

//calculates step j of md5 for message M
void md5Step(u_int M[],u_int out[],int j)
{
	u_int t;
	u_int t1;
	t=out[j]+F(out[j+3],out[j+2],out[j+1],j)+M[z[j]]+y[j];
	t1 = t>>(32-s[j]);
	t1=out[j+3]+((t<<s[j])+t1);
	out[j+4]=t1;
}

//calculates step j of md5 for message M and Mprime
void md5Step(u_int M[],u_int out[], u_int Mprime[],u_int out1[],int j)
{
	u_int t;
	u_int t1;
	t=out[j]+F(out[j+3],out[j+2],out[j+1],j)+M[z[j]]+y[j];
	t1 = t>>(32-s[j]);
	t1=out[j+3]+((t<<s[j])+t1);
	out[j+4]=t1;
	t=out1[j]+F(out1[j+3],out1[j+2],out1[j+1],j)+Mprime[z[j]]+y[j];
	t1 = t>>(32-s[j]);
	t1=out1[j+3]+((t<<s[j])+t1);
	out1[j+4]=t1;
}

//calculates phi_i
u_int F(u_int b, u_int c, u_int d, u_int i)
{
	if(i<16)
		return (b&c)|((~b)&d);
	if(i<32)
		return (b&d)|(c&(~d));	
	if(i<48)	
		return b^c^d;
	return c^(b|(~d));
}

