#include "Rewriter.h"


clang::SourceRange MyASTVisitor::getPragmaSourceRange(clang::VarDecl *vdecl,
						      clang::Rewriter &r)
{
  
  clang::SourceLocation SL = vdecl->getBeginLoc();
  clang::SourceLocation EL = vdecl->getEndLoc();
  auto &SM = r.getSourceMgr();
  unsigned Line = SM.getSpellingLineNumber(SL);
  auto FID = SM.getFileID(SL);
  return clang::SourceRange(SM.translateLineCol(FID, Line, 1),
			    EL);
}
clang::SourceRange MyASTVisitor::getPragmaSourceRange(clang::VarDecl *vdecl)
{
  return getPragmaSourceRange(vdecl, rew);
}

clang::VarDecl *MyASTVisitor::getVarDeclFromDescArray(clang::InitListExpr *IL,
						      int element)
{
  auto node_stmt = llvm::dyn_cast<clang::UnaryOperator>
    (IL->getInit(element)->IgnoreCasts());
  auto declref = llvm::dyn_cast<clang::DeclRefExpr>
    (node_stmt->getSubExpr());
  auto node_decl = declref->getDecl();
  auto VD = llvm::dyn_cast<clang::VarDecl>(node_decl);
  
  return VD;
}
