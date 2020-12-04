#include <stdio.h>
#define N1 64
#define N2 64
#pragma xmp nodes p[*][4]
#pragma xmp template t[N2][N1]
#pragma xmp distribute t[block][block] onto p
double u[N2][N1];
#pragma xmp align u[j][i] with t[j][i]

int main(int ac, char** av)
{
  
  printf("hello\n");
#pragma xmp loop (j,i) on t[j][i]  
  for(int j = 0; j < N2; j++){
    for(int i = 0;i < N1; i++){
      u[j][i] = 0.0;
    }
  }
  return 0;
}

