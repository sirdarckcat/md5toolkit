#include <stdio.h>
#include <stdlib.h>

// Reads input file of specified format and produces
// two binary files.
// The specified format is the concatenation of a valid
// output from block1 and a valid output from block2.
// The two binary files produced correspond to the two 1024-bit
// files textually represented in the outputs of block1
// and block2.
int main(int argc, char* argv[])
{
    FILE *f;
    char* line, *bc;
    unsigned int *M, *Mp;
    int i;

    M = malloc(32*sizeof(unsigned int));
    Mp = malloc(32*sizeof(unsigned int));

    if(argc < 4)
    {
        printf("usage: makeblocks INPUTFILE OUTPUTFILE1 OUTPUTFILE2\n");
        return 1;
    }

    f=fopen(argv[1], "r");
    line = malloc(256);

    bc = fgets(line, 255, f);
    i = 0;

    bc = fgets(line, 255, f);
    bc = fgets(line, 255, f);

//  Get M
    sscanf(line+ 6, "%x", &M[ 0]);
    sscanf(line+16, "%x", &M[ 1]);
    sscanf(line+26, "%x", &M[ 2]);
    sscanf(line+36, "%x", &M[ 3]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &M[ 4]);
    sscanf(line+11, "%x", &M[ 5]);
    sscanf(line+21, "%x", &M[ 6]);
    sscanf(line+31, "%x", &M[ 7]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &M[ 8]);
    sscanf(line+11, "%x", &M[ 9]);
    sscanf(line+21, "%x", &M[10]);
    sscanf(line+31, "%x", &M[11]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &M[12]);
    sscanf(line+11, "%x", &M[13]);
    sscanf(line+21, "%x", &M[14]);
    sscanf(line+31, "%x", &M[15]);
    bc = fgets(line, 255, f);
    bc = fgets(line, 255, f);



//  Get M'
    sscanf(line+ 7, "%x", &Mp[ 0]);
    sscanf(line+17, "%x", &Mp[ 1]);
    sscanf(line+27, "%x", &Mp[ 2]);
    sscanf(line+37, "%x", &Mp[ 3]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &Mp[ 4]);
    sscanf(line+11, "%x", &Mp[ 5]);
    sscanf(line+21, "%x", &Mp[ 6]);
    sscanf(line+31, "%x", &Mp[ 7]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &Mp[ 8]);
    sscanf(line+11, "%x", &Mp[ 9]);
    sscanf(line+21, "%x", &Mp[10]);
    sscanf(line+31, "%x", &Mp[11]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &Mp[12]);
    sscanf(line+11, "%x", &Mp[13]);
    sscanf(line+21, "%x", &Mp[14]);
    sscanf(line+31, "%x", &Mp[15]);
    bc = fgets(line, 255, f);
// end redo 

    bc = fgets(line, 255, f);
    bc = fgets(line, 255, f);
//  Get M
    sscanf(line+ 6, "%x", &M[16]);
    sscanf(line+16, "%x", &M[17]);
    sscanf(line+26, "%x", &M[18]);
    sscanf(line+36, "%x", &M[19]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &M[20]);
    sscanf(line+11, "%x", &M[21]);
    sscanf(line+21, "%x", &M[22]);
    sscanf(line+31, "%x", &M[23]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &M[24]);
    sscanf(line+11, "%x", &M[25]);
    sscanf(line+21, "%x", &M[26]);
    sscanf(line+31, "%x", &M[27]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &M[28]);
    sscanf(line+11, "%x", &M[29]);
    sscanf(line+21, "%x", &M[30]);
    sscanf(line+31, "%x", &M[31]);
    bc = fgets(line, 255, f);
    bc = fgets(line, 255, f);

// Get M'
    sscanf(line+ 7, "%x", &Mp[16]);
    sscanf(line+17, "%x", &Mp[17]);
    sscanf(line+27, "%x", &Mp[18]);
    sscanf(line+37, "%x", &Mp[19]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &Mp[20]);
    sscanf(line+11, "%x", &Mp[21]);
    sscanf(line+21, "%x", &Mp[22]);
    sscanf(line+31, "%x", &Mp[23]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &Mp[24]);
    sscanf(line+11, "%x", &Mp[25]);
    sscanf(line+21, "%x", &Mp[26]);
    sscanf(line+31, "%x", &Mp[27]);
    bc = fgets(line, 255, f);

    sscanf(line+ 1, "%x", &Mp[28]);
    sscanf(line+11, "%x", &Mp[29]);
    sscanf(line+21, "%x", &Mp[30]);
    sscanf(line+31, "%x", &Mp[31]);
    bc = fgets(line, 255, f);

// Write M and M' to files
    fclose(f);
    f=fopen(argv[2], "w");

    for(i=0; i<32; i++)
        fwrite(&M[i], 4, 1, f);

    fclose(f);

    f=fopen(argv[3], "w");

    for(i=0; i<32; i++)
        fwrite(&Mp[i], 4, 1, f);

    fclose(f);
    free(line);
    free(M);
    free(Mp);

    return 0;
}
