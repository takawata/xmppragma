#include <stdio.h>
#define N1 64
#define N2 64

static void *p;
/*Dimension2*/
/*Subscripts-1,4,*/
static int __XMP_NODES_SIZE_p1;
static int __XMP_NODES_RANK_p0;
static int __XMP_NODES_RANK_p1;
/*XMP_init_nodes_DYNAMIC_GLOBAL(&p,2);*/

static void *t;
/*Dimension2*/
/*Subscripts64,64,*/

/*
XMP_init_template_Chunk(t,p)
XMP_dist_template_BLOCK(t,0,0);
XMP_dist_template_BLOCK(t,1,0);
*/


double* u;
static void * _XMP_DESC_u;
/*Template t Dimension 2*/
/*Array subscripts:64,64,*/


/* found during AST traversal */
int main(int ac, char** av)
{
  /*confirm this is not replaced.*/
  {
    double u[2][3];
    u[0][0] = 0.3;
  }
 
  printf("hello\n");
/*Pragma Loop*/
{
/*Node :t*/
int j;
int _xmp_init_j= 0;
int _xmp_step_j= 1;
int _xmp_countj= 64;
int i;
int _xmp_init_i= 0;
int _xmp_step_i= 1;
int _xmp_counti= 64;

  for(j = _xmp_init_j;j <= _xmp_count_j;j += _xmp_step_j){
    for(i = _xmp_init_i;i <= _xmp_count_i;i += _xmp_step_i){
      (*(u+(i)+_XMP_GTOL_acc_u_0*(j))) = 0.0;
    }
  }
/*Reduction*/
}
  return 0;
}

