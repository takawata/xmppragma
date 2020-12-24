#include "Rewriter.h"

bool MyASTVisitor::TemplateHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	assert((content));
	auto VD = getVarDeclFromDescArray(content, 0);
	assert(VD);
	if(VD&&VD->isFunctionOrMethodVarDecl()){
		llvm::errs()<<"Local Decl\n";
	}

	VD->print(llvm::errs());
	std::string nname = VD->getName();
	int dim = content->getNumInits() - 1;
	
	{
	  clang::SourceRange SR = getPragmaSourceRange(vdecl);
	  std::string codestr;
	  llvm::raw_string_ostream ss(codestr);
	  ss<<"static ";
	  VD->print(ss);
	  ss<<";\n";
	  ss<<"/*Dimension"<< dim<<"*/\n";
	  ss<<"/*Subscripts";
	  for(int i = 1; i < content->getNumInits(); i++){
	    if( content->getInit(i)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
	      
	      ss<<ev.Val.getInt()<<",";
	    }else{
	      llvm::errs()<<"ConvErr"<<"\n";
	    }
	  }
	  ss<<"*/\n";
	  rew.ReplaceText(SR,  ss.str().c_str());
	}

	return true;
}
