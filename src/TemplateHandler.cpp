#include "Rewriter.h"

bool MyASTVisitor::TemplateHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	assert((content));
	auto VD = getVarDeclFromDescArray(content, 0);
	assert(VD);

	VD->print(llvm::errs());
	std::string nname = VD->getName();
	int dim = content->getNumInits() - 1;
	
	{
	  clang::SourceRange SR = getPragmaSourceRange(vdecl);
	  std::string codestr;
	  llvm::raw_string_ostream ss(codestr);
	  llvm::raw_string_ostream *initstream = &ss;
	  if(!VD->isLocalVarDecl()){
	    initstream = &epistream;
	  }
	  ss<<"static ";
	  VD->print(ss);
	  ss<<";\n";
	  (*initstream)<<"_XMP_init_template_FIXED(";
	  (*initstream)<<"&"<<nname<<","<<dim;
	  for(int i = 1; i < content->getNumInits(); i++){
	    if( content->getInit(i)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
	      (*initstream)<<",0 ,"<<(ev.Val.getInt()-1);
	    }else{
	      llvm::errs()<<"ConvErr"<<"\n";
	    }
	  }
	  (*initstream)<<");\n";
	  rew.ReplaceText(SR,  ss.str().c_str());
	}

	return true;
}
