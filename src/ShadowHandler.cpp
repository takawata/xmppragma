#include "Rewriter.h"

bool MyASTVisitor::ShadowHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	int dim;
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	auto TD = getVarDeclFromDescArray(content, 0);
	std::string VarName = TD->getName();
	ShadowVars.push_back(std::make_pair(TD, vdecl));
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
		llvm::raw_string_ostream codep(codestr);
		llvm::raw_string_ostream *ss = &codep;
		clang::SourceRange SR = getPragmaSourceRange(vdecl);
		llvm::errs()<<"SHADOW"<<dim<<"\n";
		if(!vdecl->isLocalVarDecl()){
		  ss = &epistream;
		}
		(*ss)<<"_XMP_init_shadow(_XMP_DESC_"<<TD->getName();
		codep<<"/*shadow*/\n";
		for(int i=0; i < dim; i++){
		  int min, max;
		  
		  if(content->getInit(2*i+2)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
		    min = ev.Val.getInt().getSExtValue();
		  }
		  
		  if(content->getInit(2*i+3)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
		    max = ev.Val.getInt().getSExtValue();
		  }

		  if(min>0 && max >0){
			  (*ss)<<",401/*_XMP_N_SHADOW_NORMAL*/,"<<min<<","<<max;
		  }

		}
		(*ss)<<");\n";
		auto it = std::find_if(Allocs.begin(), Allocs.end(),
				    [TD](auto X)
				    {return (X.aligndecl == TD);});

		if(it != Allocs.end()){
			(*ss) << getAllocString(*it);
			Allocs.erase(it);
		}else{
			llvm::errs()<<"XXXXXX";
		}
		rew.ReplaceText(SR,  codep.str().c_str());
	}
	
	return true;
	
}
