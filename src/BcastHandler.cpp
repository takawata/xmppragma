#include "Rewriter.h"

bool MyASTVisitor::BcastHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	clang::VarDecl *bcastvar;
	
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	bcastvar = getVarDeclFromDescArray(content,0);
	
	{
	  clang::SourceRange SR = getPragmaSourceRange(vdecl);
	  std::string codestr;
	  llvm::raw_string_ostream ss(codestr);
	  clang::PrintingPolicy PP(ast.getLangOpts());
	  ss<<"/*\n";
	  ss<<"*/\n";
	  ss<<"_xmp_bcast("<<bcastvar->getName()<<");";
	  rew.ReplaceText(SR,  ss.str().c_str());	  
	}

	return true;
}
