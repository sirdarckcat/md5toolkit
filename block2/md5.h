typedef unsigned int u_int;
void md5(u_int M[], u_int out[]);
void resetH();
u_int getY(int i);
u_int getZ(int i);
u_int getS(int i);
u_int getB();
void md5Step16(u_int M[],u_int vals[64]);
u_int F(u_int b, u_int c, u_int d, u_int m);
void md5Step(u_int M[],u_int out[], int j);
void md5Step(u_int M[],u_int out[], u_int Mprime[],u_int out1[],int j);
void md5Step16(u_int M[],u_int vals[64],u_int Mprime[],u_int vals1[64]);
void md5Step20(u_int M[],u_int vals[68],u_int Mprime[],u_int vals1[68]);

