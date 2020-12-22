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

/* AST traversal */
class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
  clang::Rewriter &rew;
  struct node{
    
  };
  clang::ASTContext &ast;
  std::vector<clang::VarDecl*> AlignedVars;
  bool NodeHandler(clang::VarDecl *vdecl);
  bool AlignHandler(clang::VarDecl *vdecl);
 public:
 MyASTVisitor(clang::Rewriter &r,clang::ASTContext &a) : rew(r),ast(a) {}
  /* まず、pragmaを置き換えた変数を見つける */
  bool VisitVarDecl(clang::VarDecl *vdecl);
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE);
  /* 関数宣言を見つけると呼ばれるコールバック関数 */
  bool VisitFunctionDecl(clang::FunctionDecl *fdecl);
};
