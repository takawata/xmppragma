#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>
#include <iostream>

class LoopDesc;

/* AST traversal */
class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
  clang::Rewriter &rew;
  struct LoopInfo{
    clang::VarDecl *loopVar;
    LoopDesc *LD;
    int Order;
    LoopInfo(clang::VarDecl *vd,LoopDesc *ld, int f):loopVar(vd),LD(ld),Order(f)
    {}
  };
  std::vector<LoopInfo> Loops;
  clang::ASTContext &ast;
  std::vector<clang::VarDecl*> AlignedVars;
  bool NodeHandler(clang::VarDecl *vdecl);
  bool AlignHandler(clang::VarDecl *vdecl);
  bool TemplateHandler(clang::VarDecl *vdecl);
  bool LoopHandler(clang::VarDecl *vdecl);
  bool DistributeHandler(clang::VarDecl *vdecl);
  clang::SourceRange getPragmaSourceRange(clang::VarDecl *vdecl);
public:
  static clang::SourceRange getPragmaSourceRange(clang::VarDecl *vdecl, clang::Rewriter &r);
  static clang::VarDecl *getVarDeclFromDescArray(clang::InitListExpr *, int);
  MyASTVisitor(clang::Rewriter &r,clang::ASTContext &a) : rew(r),ast(a) {}
  /* まず、pragmaを置き換えた変数を見つける */
  bool VisitVarDecl(clang::VarDecl *vdecl);
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE);
  /* 関数宣言を見つけると呼ばれるコールバック関数 */
  bool VisitFunctionDecl(clang::FunctionDecl *fdecl);
  bool VisitForStmt(clang::ForStmt *FST);
};
