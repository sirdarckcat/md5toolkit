#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>


typedef struct cond {  // doubly-linked list to store conditions
    struct cond* next;
    struct cond* prev;
    int ind;  // bit index
    int cref; // cross reference if condition is like a_ij = c_kj
    int crind; // bit index in cross reference (usually the same)
    int add_const;  // additional constant factor to add
} condition;

typedef struct nd {
    unsigned int val;  // stores the chaining variable value for a particular
                        //    step 1-48 (or 1-64 for md5)
    unsigned int Tval;
    unsigned int bf[4];  // bit-field used for quick checking/setting of conditions
    condition* list;   // pointer to list of conditions for that variable
} node;

node* Q;  // 65-68 hold initial chaining values and 69-72 hold final chaining
          // values (md5 1st block)

// differential table - holds expected differential values for each step
unsigned int dt[68];
// holds value for N[19].val - one of the main loops iterates over this value
unsigned int G_N19 = 0;

// holds step-dependent shift values for md5
int Smap5[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                  5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
                  4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                  6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
                 };

int Mmap[64] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                  1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,
                  5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,
                  0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9
                  };

// holds step-dependent constant values for md5
int Tmap[64] = {

 /* Round 1 */
   0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
   0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
   0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
   0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,

 /* Round 2 */
   0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
   0xd62f105d, 0x2441453, 0xd8a1e681, 0xe7d3fbc8,
   0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
   0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,

  /* Round 3 */
   0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
   0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
   0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x4881d05,
   0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,

  /* Round 4 */
   0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
   0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
   0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
   0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

// Function prototypes
void print_list(condition* list, int i);
void build_cond_list(char* filename, node* N);
void insert_node(node* n, int* a);
unsigned int get_bit(unsigned int x, int i);
unsigned int set_bit(unsigned int x, int i, int b);
unsigned int addsub_bit(unsigned int x, int i, int b);
unsigned int get_rbytes();
int check_cond(int ind, int print, node* N);
int wa_fb(unsigned int* IV, unsigned int* M, node* N);
void klima4_9(unsigned int* M, node* N);
int check_diffs(unsigned int* M, node* N, int index);
void first_round(unsigned int* M, node* N);
void new_randM(unsigned int* M);
int klima1_3(unsigned int *M, node* N);
unsigned int smm5(unsigned int M, int index, node* N);

// Round functions for MD5
// suggesting these inline makes code twice as fast
inline unsigned int F(unsigned int x, unsigned int y, unsigned int z)
{
    return (x & y) | (~x & z);
}

inline unsigned int G(unsigned int x, unsigned int y, unsigned int z)
{
    return (x & z) | (y & (~z));
}

inline unsigned int H(unsigned int x, unsigned int y, unsigned int z)
{
    return x ^ y ^ z;
}

inline unsigned int I(unsigned int x, unsigned int y, unsigned int z)
{
    return y ^ (x | (~z));
}

// circular left shift a 32-bit value x by s bits
inline unsigned int cls(unsigned int x, int s)
{
    return (x << s) ^ (x >> (32 - s));
}

// circular right shift a 32-bit value x by s bits
inline unsigned int crs(unsigned int x, int s)
{
    return (x >> s) ^ (x << (32 - s));
}

// returns bit value at index i (indices are 0-31 as in paper)
inline unsigned int get_bit(unsigned int x, int i)
{
    return (x >> i) & 1;
}

// sets bit i in 32-bit word x to value b (indices are 0-31 as in paper)
inline unsigned int set_bit(unsigned int x, int i, int b)
{
    if (b == 1)
        return (x | (1 << i));
    else
    {
        if(get_bit(x, i) == 0)
            return x;
        else
            return x - (1 << i);
    }
}

// set bit is sort of unnecessary with this function, but both are used
inline unsigned int addsub_bit(unsigned int x, int i, int b)
{
    return x + b*(1 << i);  // paremter b is either 1 or -1, so this works well
}

// single-message modification as presented in the Wang papers
// outputs updated M such that step computation with that M
// will yield step value with correct conditions
unsigned int smm5(unsigned int M, int index, node* N)
{
    condition* list;
    unsigned int x, y;
    int b2, i1, i2, i3, i4;


    list = N[index].list;   // list of conditions
    x = N[index].val;       // computed chaining value
    while(list != NULL)
    {
        if(list->cref < 0)     // condition of form a_i,j = 0/1
        {
                x = set_bit(x, list->ind, list->cref +2);
        }
        else        // condition of form a_i,j = b_k,l
        {
            y = N[list->cref].val;
            b2 = get_bit(y, list->crind);
            x = set_bit(x, list->ind, b2);
        }
        list = list->next;
    }
    N[index].val = x;
    i1 = index - 1;
    i2 = index - 2;
    i3 = index - 3;
    i4 = index - 4;
    if(i1 < 0)
        i1 += 68;
    if(i2 < 0)
        i2 += 68;
    if(i3 < 0)
        i3 += 68;
    if(i4 < 0)
        i4 += 68;

    // recompute correct message value for updated value of x
    return crs(x - N[i1].val, Smap5[index]) - N[i4].val -
             F(N[i1].val, N[i2].val, N[i3].val) - Tmap[index];
}

// The main iterative loop loops over values of N[19].val
// The first value of N[19].val is zero
// New values of N[19].val are computed by adding one to the previous value
// However, for some of the mmm5() code to, bits 12 and 26 of N[19].val must
// be zero.  This is why this function is necessary - to efficiently
// iterate over the space of acceptable values of N[19].val
inline void fix_n19()
{
    if(get_bit(G_N19, 12) == 1)
        G_N19 = addsub_bit(G_N19, 12, 1);
    if(get_bit(G_N19, 26) == 1)
        G_N19 = addsub_bit(G_N19, 26, 1);
    return;
}



// reads a file of a specified format and converts it into
// a list of conditions
// the file should just be a series of quintuples of numbers - one per line
// this is discussed more in README.txt
void build_cond_list(char* filename, node* N)
{
    FILE *f;
    int a[5], ind, i;
    char * line, *bc;

    f=fopen(filename, "r");
    line = malloc(256);

    bc = fgets(line, 255, f);
    
    while(line != NULL && bc != 0 && *bc != 0)
    {
        ind = 0;
        for(i=0; i<5; i++)
        {
            a[i] = atoi(line + ind);
            if(a[i] > 9 || a[i] < 0)
                ind += 3;
            else
                ind += 2;
        }
        if (a[0] != 0)  // builds data structure
            insert_node(&N[a[0]], a);

        bc = fgets(line, 255, f);
    }
    fclose(f);
    free(line);

}

// Makes bitfield of conditions so that most conditions can be checked
// by simple bitwise operations instead of traversing a list
int build_bf(node* N)
{
    int i;
    node* n;
    condition* list;
    for(i=0; i<72; i++)
    {
        n = &N[i];
        list = n->list;
        while(list != NULL)
        {
            if (list->cref == -1)
                n->bf[0] = addsub_bit(n->bf[0], list->ind, 1);
            if (list->cref == -2)
                n->bf[1] = addsub_bit(n->bf[1], list->ind, 1);
            if ((list->cref > -1) && (list->add_const == 0))
                n->bf[2] = addsub_bit(n->bf[2], list->crind, 1);
            if ((list->cref > -1) && (list->add_const != 0))
                n->bf[3] = addsub_bit(n->bf[3], list->crind, 1);
            list = list->next;
        }

    }
    return 0;
}


// Inserts node into list - this is sorted on the value of ind 
// in increasing order
void insert_node(node* n, int* a)
{
    condition *c, *list, *save;
    c = malloc(sizeof(condition));
    list = n->list;
    save = list;

    // special case to handle empty list
    if (list == NULL)
    {
        n->list = c;
        c->prev = NULL;
        c->next = NULL;
        c->ind = a[1];
        c->cref = a[2];
        c->crind = a[3];
        c->add_const = a[4];
        return;
    }

    while(list != NULL && list->ind < a[1])  // traverse down list
    {
        save = list;
        list = list->next;
    }

    // new node should be inserted in between save and list (save->next);
    if (save != list)
    {
        save->next = c;
        c->prev = save;
    }
    else
    {
        n->list = c;
        c->prev = NULL;
    }
    c->next = list;
    if(list != NULL)
        list->prev = c;
    c->ind = a[1];
    c->cref = a[2];
    c->crind = a[3];
    c->add_const = a[4];
}



// ind is just the index of the step number to be checked 
// that is - the result of that round is checked
// this of course assumes that all prior computations have been 
// saved and stored in cond_list
// returns 0 if conditions not OK, 1 otherwise
int check_cond(int ind, int print, node* N)
{
    condition* list;
    unsigned int x,y;  // stores output of round
    int b1,b2,flag=0;

    list = N[ind].list;
    x = N[ind].val;
    while(list != NULL)
    {
        // get bit value at list->ind
        b1 = get_bit(x, list->ind);
        // if list->cref is -1 or -2, then just check this value
        // -2 means bit should be zero
        // -1 means bit should be one
        if (list->cref < 0)
        {
            if (b1 != list->cref+2)
            {
                if(print)  // verbose mode
                {
                    printf("%d, %x, %d, %d, %d\n", ind, (unsigned int)x,
                            list->ind, b1, list->cref+2);
                    flag = 1;
                }
                else
                    return 0;
            }
        }
        else
        {
            // else get the bit value at the cross reference and bit index
            y = N[list->cref].val;
            b2 = get_bit(y, list->crind);
            if (b1 != (b2 ^ list->add_const))
            {
                if(print)  // verbose mode
                {
                    printf("%d, %x, %d, %d, %d, %x\n", ind, (unsigned int)x,
                            list->ind, b1, list->cref, (unsigned int)y);
                    flag = 1;
                }
                else
                    return 0;
            }
        }
        list = list->next;
    }
    if (flag == 1)
        return 0;
    return 1;
}

// used the bf field of node object to check conditions
// not really that much faster than regular check_cond() function
// returns 0 if conditions are OK, returns non-zero value otherwise
int fcheck_cond(int ind, int print, node* N)
{
    unsigned int x = 0;
    condition* list;

    x |= (~N[ind].val) & N[ind].bf[0];  
    x |= N[ind].val & N[ind].bf[1];
    x |= (N[ind-1].val & N[ind].bf[2]) ^ (N[ind].val & N[ind].bf[2]);

    if (N[ind].bf[3] != 0)
    {
        list = N[ind].list;
        while(list->add_const == 0)
            list = list->next;
        x |= (~(N[list->crind].val)&N[ind].bf[3])^(N[ind].val & N[ind].bf[2]);
    }
    if (print)
        printf("%x\n", x);
    
    return x;  //return boolean value
}

int getCV(char* s, unsigned int* CV)
{
    char string[11];

    if(strlen(s) < 32)
    {
        printf("usage: block1 [chaining value]\n");
        exit(1);
    }

    string[0] = '0';
    string[1] = 'x';
    string[10] = 0;
    strncpy(string+2, s, 8);
    sscanf(string, "%x", &CV[0]);

    strncpy(string+2, s+8, 8);
    sscanf(string, "%x", &CV[1]);

    strncpy(string+2, s+16, 8);
    sscanf(string, "%x", &CV[2]);

    strncpy(string+2, s+24, 8);
    sscanf(string, "%x", &CV[3]);

    return 0;
}

// Checks conditions and differentials
// default is to start from 0, although a significant speedup occurs
// if we start at step 20 (main iterative loop already computes steps 0-19)
// returns step number where differential was first incorrect, or -1 if all
// differentials were correct
inline int check_diffs(unsigned int* M, node* N, int index)
{
    // copy M, add differential
    unsigned int Mprime[16];
    int i;
    memcpy(Mprime,M, 16*sizeof(unsigned int));
    Mprime[ 4] = addsub_bit(Mprime[ 4], 31, 1);
    Mprime[11] = addsub_bit(Mprime[11], 15, 1);
    Mprime[14] = addsub_bit(Mprime[14], 31, 1);
    //
    // go through each round, computing step values and comparing differentials
    // we may be able to skip round 1 if mmm() code seems to work OK
    //
    if(index == 20)
    {
        for(i=15; i<20; i++)
            N[i].Tval = N[i].val + dt[i];
    }
    // Round 1 if necessary
    if(index != 20)
    {
        for(i=0; i<16; i++)
        {
            N[i].val = N[i-1].val + cls(F(N[i-1].val,N[i-2].val,N[i-3].val)
                    + N[i-4].val + M[Mmap[i]] + Tmap[i], Smap5[i]);
            N[i].Tval = N[i-1].Tval + cls(F(N[i-1].Tval,N[i-2].Tval,N[i-3].Tval)
                    + N[i-4].Tval + Mprime[Mmap[i]] + Tmap[i], Smap5[i]);

            if(N[i].Tval - N[i].val != dt[i])
                return i;
        }
        index = 16;
    }
    // Round 2
    for(i=index; i<32; i++)
    {
        N[i].val = N[i-1].val + cls(G(N[i-1].val,N[i-2].val,N[i-3].val)
                + N[i-4].val + M[Mmap[i]] + Tmap[i], Smap5[i]);
        N[i].Tval = N[i-1].Tval + cls(G(N[i-1].Tval,N[i-2].Tval,N[i-3].Tval)
                + N[i-4].Tval + Mprime[Mmap[i]] + Tmap[i], Smap5[i]);

        if(N[i].Tval - N[i].val != dt[i])
            return i;
    }
    // Round 3
    for(i=32; i<48; i++)
    {
        N[i].val = N[i-1].val + cls(H(N[i-1].val,N[i-2].val,N[i-3].val)
                + N[i-4].val + M[Mmap[i]] + Tmap[i], Smap5[i]);
        N[i].Tval = N[i-1].Tval + cls(H(N[i-1].Tval,N[i-2].Tval,N[i-3].Tval)
                + N[i-4].Tval + Mprime[Mmap[i]] + Tmap[i], Smap5[i]);

        if((i > 33) && ((N[i].Tval ^ N[i].val) != 0x80000000))
            return i;
    }
    // Round 4
    for(i=48; i<60; i++)
    {
        N[i].val = N[i-1].val + cls(I(N[i-1].val,N[i-2].val,N[i-3].val)
                + N[i-4].val + M[Mmap[i]] + Tmap[i], Smap5[i]);
        N[i].Tval = N[i-1].Tval + cls(I(N[i-1].Tval,N[i-2].Tval,N[i-3].Tval)
                + N[i-4].Tval + Mprime[Mmap[i]] + Tmap[i], Smap5[i]);

        if(N[i].Tval - N[i].val != dt[i])
            return i;
    }
    // don't worry about checking the differentials on the last four step values
    for(i=60; i<64; i++)
    {
        N[i].val = N[i-1].val + cls(I(N[i-1].val,N[i-2].val,N[i-3].val)
                + N[i-4].val + M[Mmap[i]] + Tmap[i], Smap5[i]);
        N[i].Tval = N[i-1].Tval + cls(I(N[i-1].Tval,N[i-2].Tval,N[i-3].Tval)
                + N[i-4].Tval + Mprime[Mmap[i]] + Tmap[i], Smap5[i]);
    }

    // Calculate new chaining variables
    N[68].val = N[60].val + N[-4].val;
    N[69].val = N[61].val + N[-3].val;
    N[70].val = N[62].val + N[-2].val;
    N[71].val = N[63].val + N[-1].val;
    N[68].Tval = N[60].Tval + N[-4].val;
    N[69].Tval = N[61].Tval + N[-3].val;
    N[70].Tval = N[62].Tval + N[-2].val;
    N[71].Tval = N[63].Tval + N[-1].val;

    // check diffs and conditions on chaining values
    for(i=69; i<72; i++)
    {
        if (fcheck_cond(i,0,N) != 0)
            return i;
        if (N[i].Tval - N[i].val != dt[i-4])
            return i;
    }
    return -1;  // success!
}


// code to find the first block
int wa_fb(unsigned int* IV, unsigned int* M, node* N)
{
    int stepno;

    // Store IV in appropriate data structures
    N[64].val = N[-4].val = N[-4].Tval = IV[0]; 
    N[65].val = N[-3].val = N[-3].Tval = IV[3]; 
    N[66].val = N[-2].val = N[-2].Tval = IV[2]; 
    N[67].val = N[-1].val = N[-1].Tval = IV[1]; 

    // find message such that all first round conditions and differentials
    // are satisfied - this should be fast
    first_round(M,N);

    // do the first setup steps from Klima's code (steps 1-3)
    while(klima1_3(M,N))    // sometimes klima1_3 cannot be completed for
    {                       // certain values of Q_{0-15}
        new_randM(M);
        first_round(M,N);
    }

    // iterating over possible values for N[19], check to see if all
    // other differentials/conditions hold
    klima4_9(M,N);
    stepno = check_diffs(M,N,0);

    // stepno of -1 indicates all differentials are correct for M
    while(stepno >= 0)
    {
        if(G_N19 >= 0x80000000)       // out of values of N[19]
        {
            // Pick new message and start over
            G_N19 = 0;
            while(klima1_3(M,N))  
            {
                new_randM(M);
                first_round(M,N);
            }
        }
        // iterate over values of N[19]
        klima4_9(M,N);
        stepno = check_diffs(M,N,20);
    }
    // at this point M should be of the correct form
    return 0;
}

// satisifies all first round differentials
void first_round(unsigned int* M, node* N)
{
    int diffcnt=0, flag=0, i;

    while(flag == 0)
    {
        flag = 1;
        diffcnt++;
        // round 1 code - use smm() code to find suitable first round conditions
        for(i=0; i<16; i++)
        {
            // Do initial computation
            N[i].val = N[i-1].val + cls(F(N[i-1].val,N[i-2].val,N[i-3].val)
                    + N[i-4].val + M[i] + Tmap[i], Smap5[i]);
            // perform single-message modifications
            M[i] = smm5(M[i], i, N);
            // re-comupte value from new message value
            N[i].val = N[i-1].val + cls(F(N[i-1].val,N[i-2].val,N[i-3].val)
                    + N[i-4].val + M[i] + Tmap[i], Smap5[i]);
        }
            // compute offsets to compute differentials
        M[ 4] = addsub_bit(M[ 4], 31, 1);
        M[11] = addsub_bit(M[11], 15, 1);
        M[14] = addsub_bit(M[14], 31, 1);
        for(i=0; i<16; i++)
        {
            N[i].Tval = N[i-1].Tval + cls(F(N[i-1].Tval,N[i-2].Tval,N[i-3].Tval)
                    + N[i-4].Tval + M[i] + Tmap[i], Smap5[i]);

            // If differential isn't satisfied...
            // this doesn't occur very often because the enhanced
            // conditions are *almost* sufficient, but sometimes it does
            if(N[i].Tval - N[i].val != dt[i])
            {
                flag = 0;
                new_randM(M);
            }
        }
        M[ 4] = addsub_bit(M[ 4], 31, -1);
        M[11] = addsub_bit(M[11], 15, -1);
        M[14] = addsub_bit(M[14], 31, -1);
    }
}

// picks a new random message
void new_randM(unsigned int* M)
{
    int i;

    for(i=0; i<16; i++)
        M[i] = get_rbytes();
}

// performs steps 1-3 from modified Klima code
int klima1_3(unsigned int *M, node* N)
{
    unsigned int x, y;  // these hold chaining values
    int b2, count=0;  // used to hold binary values
    condition* list;  // holds list of conditions for a particular step value

    N[17].val = 0; // initialize so check_cond fails first time
    while ((fcheck_cond(17,0, N) != 0) || (fcheck_cond(18,0, N) != 0))
    {                                                                  
        count++;
        if (count > 4096) // 4096 is set as the maximum trials
            return 1;  // conditions likely cannot be satisfied if this occurred

        // pick N[16].val arbitrarily using get_rbytes()
        N[16].val = get_rbytes();
        // fix N[16].val to correct the conditions on N[16].val
        // this takes O(1) time, then there are only a few conditions
        // on N[17] and N[18] to satisfy deterministically
        list = N[16].list;   // list of conditions
        x = N[16].val;       // computed chaining value
        while(list != NULL)
        {
            if(list->cref < 0)     // condition of form a_i,j = 0/1
            {
                x = set_bit(x, list->ind, list->cref +2);
            }
            else        // condition of form a_i,j = b_k,l
            {
                y = N[list->cref].val;
                b2 = get_bit(y, list->crind);
                x = set_bit(x, list->ind, b2);
            }
            list = list->next;
        }
        N[16].val = x;
        // compute N[17].val and N[18].val and check to see whether those conditions hold
        N[17].val = N[16].val + cls(G(N[16].val, N[15].val, N[14].val)
                + N[13].val + M[6] + Tmap[17], Smap5[17]);
        N[18].val = N[17].val + cls(G(N[17].val, N[16].val, N[15].val)
                + N[14].val + M[11] + Tmap[18], Smap5[18]);
    }
    return 0;
}

// this mmm code is based on the multi-message modification techniques that
// Klima presents in his detailed paper "Finding MD5 Collisions on a 
// Notebook PC Using Multi-message Modifications" (not the "Toy for a Notebook"
// shorter version)
// 
// The basic idea is that some of the conditions in round 2 can be satisfied. 
// Klima details how to satisfy 10 of the 15 round 2 conditions (improving
// on the 7 satisfied by Wang et. al.)  This code satisfies 13 of the 15
// round 2 conditions specified in the Wang paper, although in order to do
// this, additional round 2 conditions are necessary, so that there are actually
// 19 round 2 conditions, of which our code satisfies 17.


void klima4_9(unsigned int* M, node* N)
{
    // pick a value for N[19].val - not arbitrarily, but starting with
    // 0x00000000 and incrementing by one each iteration (use a global variable)

    N[19].val = G_N19;
    G_N19++;
    fix_n19();  // updates G_N19 to next appropriate value

    // fix this value to satisfy the conditions (one for Klima, a couple
    // more for my modifications)
    // compute M_0 as in step 5 of Klima paper
    M[0] = crs(N[19].val - N[18].val,20) - G(N[18].val,N[17].val,N[16].val)
           - N[15].val - 0xe9b6c7aa;
    // compute N[0].val using step function
    N[0].val = N[67].val + cls(M[0] + N[64].val + F(N[67].val,N[66].val,N[65].val)
               + Tmap[0], Smap5[0]);
    // compute M_1 as in step 3 of Klima paper
    M[1] = crs(N[16].val - N[15].val, 5) - G(N[15].val,N[14].val,N[13].val)
           - N[12].val - 0xf61e2562;
    // compute N[1].val using step function
    N[1].val = N[0].val + cls(M[1] + N[65].val + F(N[0].val,N[67].val,N[66].val)
               + Tmap[1], Smap5[1]);
    // compute M_2 as in step 3 of Klima paper
    M[2] = crs(N[2].val - N[1].val,17) - F(N[1].val,N[0].val,N[67].val)
           - N[66].val - Tmap[2];
    // compute M_3 as in step 3 of Klima paper
    M[3] = crs(N[3].val - N[2].val,22) - F(N[2].val,N[1].val,N[0].val)
           - N[67].val - Tmap[3];
    // compute M_4 as in step 3 of Klima paper
    M[4] = crs(N[4].val - N[3].val, 7) - F(N[3].val,N[2].val,N[1].val)
           - N[0].val - Tmap[4];
    // compute M_5 as in step 3 of Klima paper
    M[5] = crs(N[5].val - N[4].val,12) - F(N[4].val, N[3].val, N[2].val)
           - N[1].val - Tmap[5];
    N[20].val = N[19].val + cls(M[5] + N[16].val + G(N[19].val,N[18].val,N[17].val)
                + 0xd62f105d, 5);

    // New code for extra modifications ---------------------------------- //
    // The details of why this works are included in the write-up 
/*
    // Fix conditions for Q_{20}
    if (fcheck_cond(20,0,N) != 0) // conditions not satisfied for N[20].val
    {
        if(get_bit(N[20].val, 17) != get_bit(N[19].val, 17))
        {
            N[19].val = addsub_bit(N[19].val, 12, 1);
            N[18].val = addsub_bit(N[18].val, 12, 1);
            N[20].val = N[19].val + cls(M[5] + N[16].val +
                        G(N[19].val, N[18].val, N[17].val) + 0xd62f105d, 5);
        }

        if(get_bit(N[20].val, 31) == 1)
        {
            N[19].val = addsub_bit(N[19].val, 26, 1);
            N[18].val = addsub_bit(N[18].val, 26, 1);
            N[20].val = N[19].val + cls(M[5] + N[16].val +
                        G(N[19].val, N[18].val, N[17].val) + 0xd62f105d, 5);
        }
        // N[20].val should have the two conditions satisfied
        // (at least with high probability - around 15/16)

        // recompute M[11] from new value of N[18]
        M[11] = crs(N[18].val - N[17].val, Smap5[18]) -
                G(N[17].val, N[16].val, N[15].val) - N[14].val - Tmap[18];
        save = N[11].val;
        // recompute N[11].val from new value of M[11]
        N[11].val = N[10].val + cls(M[11] + N[7].val +
                    F(N[10].val, N[9].val, N[8].val) + Tmap[11], Smap5[11]);

        // recompute the rest of the step values
        M[12] = crs(N[12].val - N[11].val, Smap5[12]) -
                F(N[11].val, N[10].val, N[9].val) - Tmap[12] - N[8].val;
        M[13] = crs(N[13].val - N[12].val, Smap5[13]) -
                F(N[12].val, N[11].val, N[10].val) - Tmap[13] - N[9].val;
        M[14] = crs(N[14].val - N[13].val, Smap5[14]) -
                F(N[13].val, N[12].val, N[11].val) - Tmap[14] - N[10].val;
        M[15] = crs(N[15].val - N[14].val, Smap5[15]) -
                F(N[14].val, N[13].val, N[12].val) - Tmap[15] - N[11].val;

        // at this point N[11] could be N[11] + 2^2 + 2^{20}, which doesn't
        // mess up any conditions as
        // long as N[11] has two extra conditions
    }

    N[19].val = N[18].val + cls(M[0] + N[15].val +
                G(N[18].val, N[17].val, N[16].val) + Tmap[19], Smap5[19]);
    N[20].val = N[19].val + cls(M[5] + N[16].val +
                G(N[19].val, N[18].val, N[17].val) + 0xd62f105d, 5);
    N[21].val = N[20].val + cls(M[10] + N[17].val +
                G(N[20].val, N[19].val, N[18].val) + Tmap[21], Smap5[21]);

    // Fix conditions for Q_{21}
    // if condition not satisfied for N[21].val (bit 32 is 1)
    if (fcheck_cond(20,0,N) == 0 && fcheck_cond(21,0,N) != 0) 
    {
        N[9].val = addsub_bit(N[9].val, 22, 1);  // this bit set by default to 0
        M[9] = crs(N[9].val - N[8].val, Smap5[9])-
               F(N[8].val,N[7].val,N[6].val)-N[5].val-Tmap[9];
        M[10] = crs(N[10].val - N[9].val, Smap5[10])-
                F(N[9].val,N[8].val,N[7].val)-N[6].val-Tmap[10];
        M[12] = crs(N[12].val - N[11].val, Smap5[12]) -
                F(N[11].val, N[10].val, N[9].val) - Tmap[12] - N[8].val;
        M[13] = crs(N[13].val - N[12].val, Smap5[13]) -
                F(N[12].val, N[11].val, N[10].val) - Tmap[13] - N[9].val;
        N[21].val = N[20].val + cls(M[10] + N[17].val +
                    G(N[20].val, N[19].val, N[18].val) + Tmap[21], Smap5[21]);
    }
    */

    // More efficient way to achieve same speedup. 
    // Also discussed in the writeup
    if (fcheck_cond(20,0,N) != 0)
    {
        G_N19 += 0x7f;
        fix_n19();
        N[19].val = G_N19;
    }

    // End new code ------------------------------------------------------- //
}

void init_rbyte_gen(int a)
{
   srandom(a);
}

unsigned int get_rbytes()
{
    return random();
}


int test_md5(unsigned int* CV)
{
    unsigned int M[32];
    unsigned int IV[4]; 
    int i;

    build_cond_list("md5cond_1.txt", Q);
    build_bf(Q);

    // dt stands for 'differential table'
    // dt[i] holds the specified differential from the Wang et al paper on md5
    // for step i

    dt[0] = 0;
    dt[1] = 0;
    dt[2] = 0;
    dt[3] = 0;
    dt[4] = addsub_bit(0, 6, -1);
    dt[5] = addsub_bit(0, 6, -1);
    dt[5] = addsub_bit(dt[5], 23, 1);
    dt[5] = addsub_bit(dt[5], 31, 1);
    dt[6] = addsub_bit(0, 6, -1);
    dt[6] = addsub_bit(dt[6], 23, 1);
    dt[6] -= 1;
    dt[6] -= addsub_bit(0, 27, 1);
    dt[7] += 1;
    dt[7] -= addsub_bit(0, 15, 1);
    dt[7] -= addsub_bit(0, 17, 1);
    dt[7] -= addsub_bit(0, 23, 1);
    dt[8] += 1;
    dt[8] -= addsub_bit(0, 6, 1);
    dt[8] += addsub_bit(0, 31, 1);
    dt[9] += addsub_bit(0, 12, 1);
    dt[9] += addsub_bit(0, 31, 1);
    dt[10] += addsub_bit(0, 31, 1);
    dt[10] += addsub_bit(0, 30, 1);
    dt[11] += addsub_bit(0, 31, 1);
    dt[11] -= addsub_bit(0, 7, 1);
    dt[11] -= addsub_bit(0, 13, 1);
    dt[12] += addsub_bit(0, 24, 1);
    dt[12] += addsub_bit(0, 31, 1);
    dt[13] += addsub_bit(0, 31, 1);
    dt[14] += addsub_bit(0, 31, 1);
    dt[14] += addsub_bit(0, 3, 1);
    dt[14] -= addsub_bit(0, 15, 1);
    dt[15] += addsub_bit(0, 31, 1);
    dt[15] -= addsub_bit(0, 29, 1);
    dt[16] += addsub_bit(0, 31, 1);
    dt[17] += addsub_bit(0, 31, 1);
    dt[18] += addsub_bit(0, 31, 1);
    dt[18] += addsub_bit(0, 17, 1);
    dt[19] += addsub_bit(0, 31, 1);
    dt[20] += addsub_bit(0, 31, 1);
    dt[21] += addsub_bit(0, 31, 1);
    dt[48] += addsub_bit(0, 31, 1);
    dt[49] += addsub_bit(0, 31, -1);
    dt[50] += addsub_bit(0, 31, 1);
    dt[51] += addsub_bit(0, 31, -1);
    dt[52] += addsub_bit(0, 31, -1);
    dt[53] += addsub_bit(0, 31, -1);
    dt[54] += addsub_bit(0, 31, -1);
    dt[55] += addsub_bit(0, 31, -1);
    dt[56] += addsub_bit(0, 31, -1);
    dt[57] += addsub_bit(0, 31, -1);
    dt[58] += addsub_bit(0, 31, 1);
    dt[59] += addsub_bit(0, 31, 1);
    dt[60] += addsub_bit(0, 31, 1);
    dt[61] += addsub_bit(0, 32, 1);
    dt[61] += addsub_bit(0, 25, 1);
    dt[62] += addsub_bit(0, 31, 1);
    dt[62] += addsub_bit(0, 25, 1);
    dt[63] += addsub_bit(0, 31, -1);
    dt[63] += addsub_bit(0, 25, 1);
    dt[64] += addsub_bit(0, 31, 1);  // these last four values in dt
    dt[65] += addsub_bit(0, 31, 1);  // contain the needed differentials for the
    dt[65] += addsub_bit(0, 25, 1);  // chaining variables
    dt[66] += addsub_bit(0, 31, 1);
    dt[66] += addsub_bit(0, 25, 1);
    dt[67] += addsub_bit(0, 31, -1);
    dt[67] += addsub_bit(0, 25, 1);


    // Set the IVs - little endian right now, but I guess a message
    // found with this endianness will work in a big-endian collision
    if(CV == NULL)
    {
        Q[-4].val = IV[0] = 0x67452301;
        Q[-1].val = IV[1] = 0xefcdab89;
        Q[-2].val = IV[2] = 0x98badcfe;
        Q[-3].val = IV[3] = 0x10325476;
    }
    else
    {
        Q[-4].val = IV[0] = CV[0];
        Q[-1].val = IV[1] = CV[1];
        Q[-2].val = IV[2] = CV[2];
        Q[-3].val = IV[3] = CV[3];
    }

    // Choose initial random message
    for(i=0;i<16;i++)
        M[i] = get_rbytes();
    wa_fb(IV, M, Q);
    // re-calculates differentials
    while(check_diffs(M,Q,0) > -1)
        wa_fb(IV, M, Q);

    printf("Chaining Value for M:\n%08x%08x%08x%08x\n", Q[68].val,
            Q[71].val, Q[70].val, Q[69].val);

    // Print out the message
    printf("M = {\t");
    for(i=0; i<15; i++)
    {
        if(i%4 == 0 && i != 0)
            printf("\n\t");
        printf("%08x, ", M[i]);
    }
    printf("%08x }\n\n", M[15]);

    M[ 4] = addsub_bit(M[ 4], 31, 1);
    M[11] = addsub_bit(M[11], 15, 1);
    M[14] = addsub_bit(M[14], 31, 1);

    // Print out the message
    printf("M' = {\t");
    for(i=0; i<15; i++)
    {
        if(i%4 == 0 && i != 0)
            printf("\n\t");
        printf("%08x, ", M[i]);
    }
    printf("%08x }\n", M[15]);
    return 0;
}

int main(int argc, char* argv[])
{
    unsigned int CV[4];

    init_rbyte_gen(time(NULL) ^ (getpid() << 16));
    Q = malloc(76*sizeof(node));  
    Q = &Q[4];

    if (argc > 1)
    {
        getCV(argv[1], CV);
        test_md5(CV);
    }
    else
        test_md5(NULL);

    Q = &Q[-4];
    free(Q);
    return 0;
}


