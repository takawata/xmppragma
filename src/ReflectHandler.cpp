#include "Rewriter.h"

bool MyASTVisitor::ReflectHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;

	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	auto TD = getVarDeclFromDescArray(content, 0);
	std::string VarName = TD->getName();
	
	vdecl->print(llvm::errs());
	/*Process descriptor*/
	{
		std::string codestr;
		llvm::raw_string_ostream ss(codestr);
		clang::SourceRange SR = getPragmaSourceRange(vdecl);
		
		ss<<"_XMP_reflect__(_XMP_DESC_"<<TD->getName()<<");\n";
		rew.ReplaceText(SR,  ss.str().c_str());
	}
	
	return true;
	
}
