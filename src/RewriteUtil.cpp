#include "Rewriter.h"

std::string MyASTVisitor::getReductionFunc(int reductionType)
{
	std::string reductionFunc;
	switch(reductionType){
	case clang::tok::plus:
		reductionFunc ="sum";
		break;
	case clang::tok::star:
		reductionFunc ="mult";
		break;
	case clang::tok::amp:
		reductionFunc ="and";
		break;
	case clang::tok::ampamp:
		reductionFunc ="land";
		break;
	case clang::tok::pipe:
		reductionFunc ="or";
		break;
	case clang::tok::pipepipe:
		reductionFunc ="lor";
		break;
	case -1:
		reductionFunc ="max";
		break;
	case -2:
		reductionFunc ="min";
		break;
	case -3:
		reductionFunc ="firstmax";
		break;
	case -4:
		reductionFunc ="firstmin";
		break;
	case -5:
		reductionFunc ="lastmax";
		break;
	case -6:
		reductionFunc ="lastmin";
		break;
	}
	return reductionFunc;
}

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
