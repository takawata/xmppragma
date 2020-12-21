#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Version.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
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
  struct align_vars{
    std::vector<clang::VarDecl*> vars;
  };
  bool NodeHandler(clang::VarDecl *vdecl);
 public:
 MyASTVisitor(clang::Rewriter &r,clang::ASTContext &a) : rew(r),ast(a) {}
  /* まず、pragmaを置き換えた変数を見つける */
  bool VisitVarDecl(clang::VarDecl *vdecl);
  /* 関数宣言を見つけると呼ばれるコールバック関数 */
  bool VisitFunctionDecl(clang::FunctionDecl *fdecl);
};
