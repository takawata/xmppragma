#include "Rewriter.h"

bool MyASTVisitor::ShadowHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	int dim;
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	auto TD = getVarDeclFromDescArray(content, 0);
	std::string VarName = TD->getName();
	if(content->getNumInits() < 2){
	  llvm::errs()<<"Invalid descriptor\n";
	}
	content->getInit(1)->dump();
	if(content->getInit(1)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
	  dim = ev.Val.getInt().getSExtValue();
	}
	vdecl->print(llvm::errs());
	/*Process descriptor*/
	{
		std::string codestr;
		llvm::raw_string_ostream ss(codestr);
		clang::SourceRange SR = getPragmaSourceRange(vdecl);
		llvm::errs()<<"SHADOW"<<dim<<"\n";
		ss<<"/*\n";
		for(int i=0; i < dim; i++){
		  int min, max;
		  
		  if(content->getInit(2*i+2)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
		    min = ev.Val.getInt().getSExtValue();
		  }
		  
		  if(content->getInit(2*i+3)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
		    max = ev.Val.getInt().getSExtValue();
		  }
		  
		  ss<<"xmp_shadow(_XMP_DESC_"<<TD->getName()<<","
		    <<i<<","<<min<<","<<max<<");\n";
		  
		}
		ss<<"*/\n";		
		rew.ReplaceText(SR,  ss.str().c_str());
	}
	
	return true;
	
}
