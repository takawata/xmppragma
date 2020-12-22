#include "Rewriter.h"

bool MyASTVisitor::AlignHandler(clang::VarDecl *vdecl)
{
  auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
  assert((content));
  auto align_stmt = llvm::dyn_cast<clang::UnaryOperator>
    (content->getInit(0)->IgnoreCasts());
  auto declref = llvm::dyn_cast<clang::DeclRefExpr>
    (align_stmt->getSubExpr());
  auto align_decl = declref->getDecl();
  align_decl->dump();
  auto avdecl = llvm::dyn_cast<clang::VarDecl>(align_decl);
  assert(avdecl);
  AlignedVars.push_back(avdecl);
  rew.InsertTextBefore(vdecl->getBeginLoc(),
		       "/* Pragma Align Found */");
  return true;
}
