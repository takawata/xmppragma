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

Alignが認識されていて、コメントが追加されていること

u[i][j]が (*(_XMP_ADDR_u+(i)+_XMP_GTOL_acc_u_0*(j)))に置き換えられていること

その上の、別のスコープにしてあるu[0][0]が置き換えられていないこと

