% mkdir build
% cd build
% cmake ../
% make
% ./xmppragma ../test/test.cpp > result.cpp
% diff result.cpp ../test/result.cpp
(差分がないことを確認)

手元の標準出力をtest/result.cppに出している。
コンパイラのエラーが起きないことを確認し
作業中でコードに反映されないプリプロセッサの処理の確認を行うために構文木のダンプを行っている。

着目点は

#pragma node がresult.cppでは消えていて、
void *p;
static int __XMP_NODES_SIZE_p1;
static int __XMP_NODES_RANK_p0;
static int __XMP_NODES_RANK_p1;
/*XMP_init_nodes_DYNAMIC_GLOBAL(&p,2);*/
に置き換えられていること。

#pragma nodeが複数行に渡っているが正しく処理されていること

配列定義のdouble u[N2][N1]がdouble *uに置き換わっていること

#pragma xmp alignが
static void * _XMP_DESC_u;
/*Template t Dimension 2*/
/*Array subscripts:64,64,*/

に置き換わっていること

u[i][j]が (*(_XMP_ADDR_u+(i)+_XMP_GTOL_acc_u_0*(j)))に置き換えられていること

その上の、別のスコープにしてあるu[0][0]が置き換えられていないこと

#pragma xmp template t[64][64]が

static void *t;
/*Dimension2*/
/*Subscripts64,64,*/

に書き換えられていること。

