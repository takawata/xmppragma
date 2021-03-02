#include "Rewriter.h"

bool MyASTVisitor::NodeHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	llvm::raw_string_ostream *initstream;

	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	assert((content));
	llvm::errs()<<"NODE"<<"\n";
	auto VD = getVarDeclFromDescArray(content, 0);
	assert(VD);
	VD->print(llvm::errs());
	std::string nname = VD->getName();
	int dim = content->getNumInits() - 1;
	{
	  clang::SourceRange SR = getPragmaSourceRange(vdecl);
	  std::string codestr;
	  llvm::raw_string_ostream ss(codestr);
	  initstream = &ss;
	  if(!VD->isFunctionOrMethodVarDecl()){
	    initstream = &epistream;
	  }
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
	  for(int i = 1; i < dim; i++){
		  ss<<"static int __XMP_NODES_SIZE_"<<nname<<i<<";\n";
	  }

	  for(int i = 0; i < dim; i++){
		  ss<<"static int __XMP_NODES_RANK_"<<nname<<i<<";\n";
	  }
	  (*initstream)<<"/*";
	  (*initstream)<<"XMP_init_nodes_DYNAMIC_GLOBAL(&"<<nname<<","<< dim <<");";
	  (*initstream)<<"*/"<<"\n";
	  rew.ReplaceText(SR,  ss.str().c_str());
	}
	return true;
}
