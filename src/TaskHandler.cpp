#include "Rewriter.h"

bool MyASTVisitor::TaskHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	clang::VarDecl *taskvar;
	
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	taskvar = getVarDeclFromDescArray(content,0);
	
	{
	  clang::SourceRange SR = getPragmaSourceRange(vdecl);
	  std::string codestr;
	  llvm::raw_string_ostream ss(codestr);
	  clang::PrintingPolicy PP(ast.getLangOpts());
	  ss<<"/*\n";
	  ss<<"*/\n";
	  taskvar->print(ss);
	  ss<<";\n";
	  ss<<"if(__xmp_task_match("<<taskvar->getName()<<"))";
	  rew.ReplaceText(SR,  ss.str().c_str());	  
	}

	return true;
}
