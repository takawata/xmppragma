#include <stdio.h>
#define N1 64
#define N2 64

static void *p;
static int __XMP_NODES_SIZE_p1;
static int __XMP_NODES_RANK_p0;
static int __XMP_NODES_RANK_p1;
/*XMP_init_nodes_DYNAMIC_GLOBAL(&p,2);*/

#pragma xmp template t[N2][N1]
#pragma xmp distribute t[block][block] onto p

double* u;
static void * _XMP_DESC_u;

/* found during AST traversal */
int main(int ac, char** av)
{
  /*confirm this is not replaced.*/
  {
    double u[2][3];
    u[0][0] = 0.3;
  }
 
  printf("hello\n");
#pragma xmp loop (j,i) on t[j][i]  
  for(int j = 0; j < N2; j++){
    for(int i = 0;i < N1; i++){
      (*(u+(i)+_XMP_GTOL_acc_u_0*(j))) = 0.0;
    }
  }
  return 0;
}

