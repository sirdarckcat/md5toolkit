#include "md5.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <time.h>
#define phi(i) F(Q[i-1],Q[i-2],Q[i-3],i-4)
using namespace std;

// Date: 12-30-2005

/* 
  Implementation of the Wang et. al. attacks on MD family of hash functions
	
  This code aims to produce the 2nd block of an MD5 collisions using the attack
  found in the papers presented by Wang at the 2005 Eurocrypt conference
  ("Cryptanalysis of the Hash Functions MD4 and RIPEMD" and "How to Break MD5 
  and Other Hash Functions").

  All of the single message modification is done by producing chaining values 
  Q[0-15], which satisfy the conditions for these chaingin values.  The message
  by then be calculated from the chaining values.
	
*/


typedef unsigned int u_int;


u_int RR(u_int var, int num);
u_int RL(u_int var, int num);
void readfile();
void findx(u_int Q[24],u_int M[16],u_int Mprime[16]);
void satisfy_stationary(u_int Q[24],int type1);
void printData(u_int M[16], u_int Mprime[16], u_int Q[68], u_int Qprime[68]);
bool multiMessage2(u_int M[16],u_int Mprime[16],u_int Q[68], u_int Qprime[68]);
bool check_stationary(u_int Q[68],int k,int m);
void readCV(u_int* CV);

/*
xor differences between M and M' for each step 
*/
u_int differences [] = {
	0x82000000,0x82000020,0xfe3f18e0,0x8600003e,
	0x80001fc1,0x80330000,0x980003c0,0x87838000,
	0x800003c3,0x80001000,0x80000000,0x800fe080,
	0xff000000,0x80000000,0x80008008,0xa0000000,
	0x80000000, 0x80000000, 0x80020000, 0x80000000,
	0x80000000, 0x80000000, 0, 0,
	0,0,0,0,
	0,0,0,0,
	0,0,0x80000000, 0x80000000,
	0x80000000, 0x80000000, 0x80000000, 0x80000000,
	0x80000000, 0x80000000, 0x80000000, 0x80000000,
	0x80000000, 0x80000000, 0x80000000, 0x80000000,
	0x80000000, 0x80000000, 0x80000000, 0x80000000,
	0x80000000, 0x80000000, 0x80000000, 0x80000000,
	0x80000000, 0x80000000, 0x80000000, 0x80000000,
	0x80000000, 0x82000000, 0x82000000, 0x86000000
};

int NUMCV = 1;
int NUMCOLLISION = 1;
const int DEBUG = 0;

//array that contains the list of conditions
//the conditions can be found in md5cond2.txt
u_int cond[309][3];

int main(int argc, char** argv)
{
	if(argc>1)
		NUMCV = atoi(argv[1]);
	if(argc>2)
		NUMCOLLISION = atoi(argv[2]);
	
	u_int totalTime =(unsigned)time( NULL );

	//arrays of chaining values of M and M'
	u_int Q[68];	
	u_int Qprime[68];
	u_int* cvList = (u_int*) malloc(4*NUMCV * sizeof(u_int));
	
	readfile();
	readCV(cvList);

	//cycle through the NUMCV sets of Chaining Values in the CV.txt
	for(int loop=0; loop<NUMCV; loop++)
	{
	
		/*
		Q[0-3] are the Chaining Values from the first block.
		These values are read in from the file CV.txt 
		The values should be placed in the file in the order
		H1 H2 H3 H4.
		*/
	
		Q[0] = cvList[4*loop];
		Q[1] = cvList[4*loop+1];
		Q[2] = cvList[4*loop+2];
		Q[3] = cvList[4*loop+3];
		
		/*
		Set Qprime to have appropriate differentials
		*/
		
		Qprime[0] = Q[0]^(0x80000000);
		Qprime[1] = Q[1]^(0x82000000);
		Qprime[2] = Q[2]^(0x86000000);
		Qprime[3] = Q[3]^(0x82000000);
		
		if(DEBUG)
		{
			if(	(Q[3] & 0x06000020) != 0x0
			||	(Q[2] & 0x06000000) != 0x02000000
			||	(Q[1] & 0x02000000) != 0x0
			||  	(Q[3] & 0x80000000) != (Q[2] & 0x80000000)
			||  	(Q[2] & 0x80000000) != (Q[1] & 0x80000000) 	)
			{
				printf("%x %x %x %x %x %x\n", Q[3] & 0x06000020, Q[2] & 0x06000000, Q[1] & 0x02000000, Q[3], Q[2], Q[1]);
				printf("conditions on chaining values not satisfied\n");
				continue;
			}
		}
		
		//Find NUMCOLLISION collisions for each chaining value
		for(int loop2 = 0; loop2<NUMCOLLISION; loop2++)
		{
			bool messageFound = false;
			u_int collTime =(unsigned)time( NULL );
			while(!messageFound)
			{
				if(DEBUG)
				{
					printf("beginning to satisfy conditions through step 20.\n");
				}
				//seed random number generator
				srand( (unsigned)time( NULL ) );
				srand(rand());
			
				bool b =true;
				bool c =true;
			
				//2nd block messages M and M'
				u_int M [16];
				u_int Mprime [16];
			
			
				/*
				finds a block that satisfies differentials and conditions
				through step value Q[15]
				*/
				while(c)
				{
					b=true;
					while(b)
					{
						for(int i=4; i<20;i++)
						{	
							Q[i]=rand()+(rand()<<16);
						}
						satisfy_stationary(Q,1);
						findx(Q,M,Mprime);
						if(((M[4]|M[14])&0x80000000)!=0&&(M[11]&0x8000)!=0)
						{
							md5Step20(M,Q,Mprime,Qprime);
							if(((Q[19]^Qprime[19])==0xa0000000))
							{
								if(check_stationary(Q,0,274))
									b=false;
							}
						}
					}
			
					/*
					modify Q[0] and Q[1] until conditions and differentials
					are satisfied through Q[20]. If this code can not find a
					message within 0x10000 find another message with conditions
					for Q[2-15] satisfied.
					*/
					b=true;
					int number=0;
					while(b)
					{
						number++;
						// randomly select Q[0,1]
						Q[5]=rand()+(rand()<<16);
						Q[4]=rand()+(rand()<<16);
			
						//satisfy conditions for Q[0] and Q[1]
						satisfy_stationary(Q,2);
						findx(Q,M,Mprime);
						md5Step20(M,Q,Mprime,Qprime);
						if(number == 0x10000)
							b=false;
						if(	((Q[19]^Qprime[19])==0xa0000000)
						&&	((Q[24]^Qprime[24])==0x80000000)
						&&	check_stationary(Q,0,286)	)
						{
							c=false;
							b=false;
						}
					}
				}
				
				if(DEBUG)
				{
					printf("satisfied conditions through step 20.\n");
				}
				
				//perform probabilistic search for a collision
				messageFound=multiMessage2(M,Mprime,Q,Qprime);
			
				//output time taken to find collision
				if(messageFound)
				{
//					printf("time %d seconds\n",(unsigned)time(NULL)-collTime);
//					printf("chaining values ");
//					printf("aa0 %x ",Q[0]);
//					printf("bb0 %x ", Q[3]);
//					printf("cc0 %x ", Q[2]);
//					printf("dd0 %x \n", Q[1]);
					printData(M,Mprime,Q,Qprime);
//					printf("\n");
				}
			}
		}
	}
//	printf("total execution time %d seconds\n",(unsigned)time(NULL)-totalTime);
}


/*
modifies chaining values so that specified conditions are satisfied
*/

void satisfy_stationary(u_int Q[24],int type1)
{
	int bit;
	int type;
	int j=0;
	int k;
	int m;

	//satisfy Q[7-10] for multimessage
	if(type1==0)
	{
		m=145;
		k=211;
	}
	//satisfy Q[0,1] 
	else if(type1==2)
	{
		m=0;
		k=52;
	}
	//satisfy Q[0-15]
	else {
		m=0;
		k=274;
	}
	//reads through conditions modifying Q[0-15] to satisfy their conditions
	for(int i=m; i<k; i++)
	{
		j=cond[i][0]+4;
		u_int zeroBit = 0xffffffff;
		u_int oneBit = 0;
		while(cond[i][0]==j-4)
		{
			bit=cond[i][1];
			type=cond[i][2];
			//designated bit should be set to zero
			if(type == 0)
			{
				zeroBit = zeroBit & ~(1<<(bit-1));
			}
			//designated bit should be set to one
			else if(type == 1)
			{
				oneBit = oneBit | (1<<(bit-1));
			}
			/*designated bit should be set eQual to the
			same bit of the previous chaining value*/
			else if(type == 2)
			{
				if((Q[j-1]&(1<<(bit-1)))!=0)
					oneBit = oneBit | (1<<(bit-1));
				else
					zeroBit = zeroBit & ~(1<<(bit-1));
			}
			/*designated bit in chaining value x should 
			be set eQual to the same bit of chaining value x-2*/
			else if(type == 3)
			{
				if((Q[j-2]&(1<<(bit-1)))!=0)
					oneBit = oneBit | (1<<(bit-1));
				else
					zeroBit = zeroBit & ~(1<<(bit-1));
			}
			/*designated bit should be set to the negation
			of the same bit of the previous chaining value*/
			else if(type == 4)
			{
				//printf("here");
				if((Q[j-1]&(1<<(bit-1)))==0)
					oneBit = oneBit | (1<<(bit-1));
				else
					zeroBit = zeroBit & ~(1<<(bit-1));
			}
			i++;
		}
		i--;
		//modify Q[j] to satisfy conditions
		Q[j] = Q[j] | oneBit;
		Q[j] = Q[j] & zeroBit;
		
	}
}

/*
Reads through specifed conditions and returns true if the are
all satisfied, otherwise returns false.
similar to satisfy_stationary
*/

bool check_stationary(u_int Q[68],int m,int k)
{
	int bit;
	int type;
	int j=0;
	for(int i=m; i<k; i++)
	{
		j=cond[i][0]+4;
		u_int zeroBit = 0xffffffff;
		u_int oneBit = 0;
		while(cond[i][0]==j-4)
		{
			bit=cond[i][1];
			type=cond[i][2];
			if(type == 0)
			{
				zeroBit = zeroBit & ~(1<<(bit-1));
			}
			else if(type == 1)
			{
				oneBit = oneBit | (1<<(bit-1));
			}
			else if(type == 2)
			{
				if((Q[j-1]&(1<<(bit-1)))!=0)
					oneBit = oneBit | (1<<(bit-1));
				else
					zeroBit = zeroBit & ~(1<<(bit-1));
			}
			else if(type == 3)
			{
				if((Q[j-2]&(1<<(bit-1)))!=0)
					oneBit = oneBit | (1<<(bit-1));
				else
					zeroBit = zeroBit & ~(1<<(bit-1));
			}
			else if(type == 4)
			{
				if((Q[j-1]&(1<<(bit-1)))==0)
					oneBit = oneBit | (1<<(bit-1));
				else
					zeroBit = zeroBit & ~(1<<(bit-1));
			}
			i++;
		}
		i--;
		if(Q[j] != (Q[j] | oneBit))
		{
			//printf("%d %x 1\n", j, Q[j]);
			return false;
		}
		if(Q[j] != (Q[j] & zeroBit))
		{
			//printf("%d %x 2\n", j, Q[j]);
			return false;
		}
	}
	return true;
}

/*
performs steps 4-7 outlined in the paper
Probabilistic portion of attack.
*/

bool multiMessage2(u_int M[16],u_int Mprime[16],u_int Q[68], u_int Qprime[68])
{
	srand( (unsigned)time( NULL ) );
	srand(rand());

	u_int time1 =(unsigned)time( NULL );
	for(u_int i=1;i<0x1000;i++)
	{
		//if time exceeds 40 seconds return
		//this value is subject to changed based on the speed of the system
		if(((unsigned)time( NULL )-time1)>=40)
		{
			if(DEBUG)
			{
				printf("no collision   stepped through %x steps\n", i);
			}
			return false;
		}
		
		//makes the conditions for the while loop unsatisfied
		Qprime[19]=0;

		while(	((Q[24]^Qprime[24])!=0x80000000)
		||		((Q[19]^Qprime[19])!=0xa0000000)	)
		{
			//randomly select Q[7-10] and satisfy conditons
			Q[11]=(((rand()+(rand()<<16))&0xe47efffe)|0x843283c0);
			//sets Q[7]_2 = Q[6]_2
			if((Q[10]&0x2)==0)
				Q[11]=Q[11]&0xfffffffd;
			else Q[11]=Q[11]|0x2;
			Q[12]=(((rand()+(rand()<<16))&0xfc7d7dfd)|0x9c0101c1);
			if((Q[11]&0x1000)==0)
				Q[12]=Q[12]&0xffffefff;
			else Q[12]=Q[12]|0x1000;
			Q[13]=(((rand()+(rand()<<16))&0xfffbeffc)|0x878383c0);
			Q[14]=(((rand()+(rand()<<16))&0xfffdefff)|0x800583c3);
			if((Q[13]&0x80000)==0)
				Q[14]=Q[14]&0xfff7ffff;
			else Q[14]=Q[14]|0x80000;
			if((Q[13]&0x4000)==0)
				Q[14]=Q[14]&0xffffbfff;
			else Q[14]=Q[14]|0x4000;
			if((Q[13]&0x2000)==0)
				Q[14]=Q[14]&0xffffdfff;
			else Q[14]=Q[14]|0x2000;
			if((Q[10]&0x80000000)==0)
			{
				Q[11]=Q[11]&0x7fffffff;
				Q[12]=Q[12]&0x7fffffff;
				Q[13]=Q[13]&0x7fffffff;
				Q[14]=Q[14]&0x7fffffff;
			}


			//calculate Q[11]
			Q[15] = Q[14] + RL(phi(15) + 0x895cd7be + M[11]+Q[11],22);
			
			if(	(Q[15]&0xfff81fff)==Q[15]
			&&	(Q[15]|0x00081080)==Q[15]
			&&	((Q[14]^Q[15])&0xff000000)==0	)
			{
				for(int i=7;i<16;i++)
				{ 
					M[i]=RR(Q[i+4]-Q[i+3],getS(i)) - getY(i) - Q[i] - phi(i+4);
				}
				for(int v=7;v<16;v++)
				{
					Mprime[v]=M[v];  
				}
				Mprime[11]=Mprime[11]-0x8000;
				Mprime[14]=Mprime[14]-0x80000000;
				md5Step20(M,Q,Mprime,Qprime);
			}
		}

		/*
		truth is true when all conditions and diffenerentials are satisfied to
		the point that has been checked
		*/
		
		bool truth =true;
		/*
		this for loops uses gray code to cycle through all of the possible bit
		changes to Q[9] and Q[10] that do not affect Q[11] or x[11]
		*/
		u_int x11 = Q[15];
		for(u_int j=0; j<0x20000;j++)
		{
			
			truth=true;
			
			//flip bits using gray code
			if((j&0x1)!=0)	
			{
				if((Q[14]&0x4)==0)
					Q[13]=Q[13]^0x4;
				else Q[12]=Q[12]^0x4;
			}
			else if((j&0x2)!=0)
			{
				if((Q[14]&0x8)==0)
					Q[13]=Q[13]^0x8;
				else Q[12]=Q[12]^0x8;
			}
			else if((j&0x4)!=0)
			{
				if((Q[14]&0x10)==0)
					Q[13]=Q[13]^0x10;
				else Q[12]=Q[12]^0x10;
			}
			else if((j&0x8)!=0)
			{
				if((Q[14]&0x20)==0)
					Q[13]=Q[13]^0x20;
				else Q[12]=Q[12]^0x20;
			}
			else if((j&0x10)!=0)
			{
				if((Q[14]&0x400)==0)
					Q[13]=Q[13]^0x400;
				else Q[12]=Q[12]^0x400;
			}
			else if((j&0x20)!=0)
			{
				if((Q[14]&0x800)==0)
					Q[13]=Q[13]^0x800;
				else Q[12]=Q[12]^0x800;
			}
			else if((j&0x40)!=0)
			{
				if((Q[14]&0x100000)==0)
					Q[13]=Q[13]^0x100000;
				else Q[12]=Q[12]^0x100000;
			}
			else if((j&0x80)!=0)
			{
				if((Q[14]&0x200000)==0)
					Q[13]=Q[13]^0x200000;
				else Q[12]=Q[12]^0x200000;
			}
			else if((j&0x100)!=0)
			{
				if((Q[14]&0x400000)==0)
					Q[13]=Q[13]^0x400000;
				else Q[12]=Q[12]^0x400000;
			}
			else if((j&0x200)!=0)
			{
				if((Q[14]&0x20000000)==0)
					Q[13]=Q[13]^0x20000000;
				else Q[12]=Q[12]^0x20000000;
			}
			else if((j&0x400)!=0)
			{
				if((Q[14]&0x40000000)==0)
					Q[13]=Q[13]^0x40000000;
				else Q[12]=Q[12]^0x40000000;
			}
			else if((j&0x800)!=0)
			{
				if((Q[14]&0x4000)==0)
					j=j+0x7ff;
				else Q[12]=Q[12]^0x4000;
			}
			else if((j&0x1000)!=0)
			{
				if((Q[14]&0x80000)==0)
					j=j+0xfff;
				else Q[12]=Q[12]^0x80000;
			}
			else if((j&0x2000)!=0)
			{
				if((Q[14]&0x40000)==0)
					j=j+0x1fff;
				else Q[12]=Q[12]^0x40000;
			}
			else if((j&0x4000)!=0)
			{
				if((Q[14]&0x8000000)!=0)
					j=j+0x3fff;
				else Q[13]=Q[13]^0x8000000;
			}
			else if((j&0x8000)!=0)
			{
				if((Q[14]&0x10000000)!=0)
					j=j+0x7fff;
				else Q[13]=Q[13]^0x10000000;
			}
			else if((j&0x10000)!=0)
			{
				if((Q[14]&0x2000)==0)
					j=j+0xffff;
				else Q[12]=Q[12]^0x2000;
			}
			
			//recalculate Q[8-14]
			for(int p=8;p<14;p++)
			{
				M[p] = RR(Q[p+4]-Q[p+3],getS(p)) - getY(p) - Q[p] - phi(p+4);
				Mprime[p] = M[p];
			}
			Mprime[11]=Mprime[11]-0x8000;
			// calculate md5 of message while checking differnces
			md5Step20(M,Q,Mprime,Qprime);
			int k;
			for(k=21;k<64;k++)
			{
				md5Step(M,Q,Mprime,Qprime,k);
				if((Q[k+4]^Qprime[k+4])!=differences[k])
				{
					truth =false;
					break;
				}
			}


			//If conditions are all satisfied check to see if hashes collide
			if(truth)
			{
				u_int val64=Q[64]+Q[0];
				u_int val65=Q[65]+Q[1];
				u_int val66=Q[66]+Q[2];
				u_int val67=Q[67]+Q[3];
				u_int val164=Qprime[64]+Qprime[0];
				u_int val165=Qprime[65]+Qprime[1];
				u_int val166=Qprime[66]+Qprime[2];
				u_int val167=Qprime[67]+Qprime[3];
				
				if(	(val64^val164)==0
				&&	(val65^val165)==0
				&&	(val66^val166)==0
				&&	(val67^val167)==0	)
				{
					return true;
				}
			}
			
		}
	}
	return false;
}


/*
Given Q[0-15] calculates M[0-15] and Mprime[0-15]
*/
void findx(u_int Q[24],u_int M[16],u_int Mprime[16])
{
	for(int i=4;i<20;i++)
	{
		M[i-4]= RR((Q[i]-Q[i-1]),getS(i-4)) - getY(i-4) - Q[i-4] - phi(i);
		Mprime[i-4] = M[i-4];
	}
	Mprime[4]=Mprime[4]-0x80000000;
	Mprime[11]=Mprime[11]-0x8000;
	Mprime[14]=Mprime[14]-0x80000000;
}

/*
void printData(u_int M[16], u_int Mprime[16], u_int Q[68], u_int Qprime[68])
{
	printf("aa1 %x %x\n",(Q[64]+Q[0]),(Qprime[64]+Qprime[0]));
	printf("dd1 %x %x\n",(Q[65]+Q[1]),(Qprime[65]+Qprime[1]));
	printf("cc1 %x %x\n",(Q[66]+Q[2]),(Qprime[66]+Qprime[2]));
	printf("bb1 %x %x\n",(Q[67]+Q[3]),(Qprime[67]+Qprime[3]));
	for(int i=0; i<16; i++)
	{
		printf("%x %x %x\n",i, M[i],Mprime[i]);
	}
}
*/
// prints all chaining values, final hash, and M[0-15]
void printData(u_int M[16], u_int Mprime[16], u_int Q[68], u_int Qprime[68])
{
    int i;

//	printf("aa1 %x\n",(Q[0]));
//	printf("dd1 %x\n",(Q[1]));
//	printf("cc1 %x\n",(Q[2]));
//	printf("bb1 %x\n",(Q[3]));

    printf("Chaining Value for M:\n%08x%08x%08x%08x\n", Q[64]+Q[0], Q[67]+Q[3], Q[66]+Q[2], Q[65]+Q[1]);
    // Print out the message
    printf("M = {\t");
    for(i=0; i<15; i++)
    {
        if(i%4 == 0 && i != 0)
            printf("\n\t");
        printf("%08x, ", M[i]);
    }
    printf("%08x }\n\n", M[15]);

    // Print out the message
    printf("M' = {\t");
    for(i=0; i<15; i++)
    {
        if(i%4 == 0 && i != 0)
            printf("\n\t");
        printf("%08x, ", Mprime[i]);
    }
    printf("%08x }\n\n", Mprime[15]);

}

/*
reads input file containg condition and fills array with data
*/

void readfile()
{
	std::ifstream input("md5cond2.txt"); 
	for(int i=0;i<309;i++)
	{
		input >> cond[i][0];
		input >> cond[i][1];
		input >> cond[i][2];
	}
}

void readCV(u_int* CV)
{
	std::ifstream input("CV.txt"); 
    char string[11];
    char line[256];
    int i=0;

    string[0] = '0';
    string[1] = 'x';
    string[10] = 0;

    for(i=0; i<NUMCV; i++)
    {

        input >> line;
        strncpy(string+2, line, 8);
        sscanf(string, "%x", &CV[4*i+0]);

        strncpy(string+2, line+8, 8);
        sscanf(string, "%x", &CV[4*i+3]);

        strncpy(string+2, line+16, 8);
        sscanf(string, "%x", &CV[4*i+2]);

        strncpy(string+2, line+24, 8);
        sscanf(string, "%x", &CV[4*i+1]);

    }

}

/*
rotates a 32 bit word to the right num digits
*/

u_int RR(u_int var, int num)
{
	u_int temp = var >> num;
	return (var << (32-num))|temp;
}

/*
rotates a 32 bit word to the left num digits
*/

u_int RL(u_int var, int num)
{
	u_int temp = var << num;
	return (var >> (32-num))|temp;
}
