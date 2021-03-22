#include "Rewriter.h"

bool MyASTVisitor::NodeHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	llvm::raw_string_ostream *initstream;
	std::vector<int> subscripts;

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

	  for(int i = 0; i < dim; i++){
	    if(!content->getInit(i+1)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
	      content->getInit(i+1)->dump();
	      llvm::errs()<<"XX";
	      exit(-1);
	    }
	    subscripts.push_back(ev.Val.getInt().getSExtValue());
	  }
	  ss<<"/*Dimension"<< dim<<"*/\n";
	  ss<<"/*Subscripts";
	  ss<<"*/\n";
	  for(int i = 0; i < dim; i++){
	    ss<<"static ";
	    if(subscripts[i] != -1){
	      ss <<"const ";
	    }
	    ss<<"int __XMP_NODES_SIZE_"<<nname<<(dim-i-1);
	    if(subscripts[i] != -1){
	      ss <<" = " << subscripts[i];
	    }
	    ss <<";\n";
	  }

	  for(int i = 0; i < dim; i++){
		  ss<<"static int __XMP_NODES_RANK_"<<nname<<i<<";\n";
	  }
	  (*initstream)<<"_XMP_init_nodes_DYNAMIC_GLOBAL(&"<<nname<<","<< dim <<"";
	  for(int i = 0; i < dim; i++){
	    (*initstream)<<","<<subscripts[dim - i -1];
	    if(subscripts[dim - i - 1] == -1){
	      (*initstream)<<", & __XMP_NODES_SIZE_"<<nname<<i;
	    }
	  }
	  for(int i = 0; i < dim; i++){
	    (*initstream)<<", &__XMP_NODES_RANK_"<<nname<<i;
	  }

	  (*initstream)<<");\n";
	  rew.ReplaceText(SR,  ss.str().c_str());
	}
	return true;
}
