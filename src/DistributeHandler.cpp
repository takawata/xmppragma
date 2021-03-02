#include "Rewriter.h"

bool MyASTVisitor::DistributeHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	llvm::raw_string_ostream *initstream;
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	auto TD = getVarDeclFromDescArray(content, 0);
	auto ND = getVarDeclFromDescArray(content, 1);
	
	vdecl->print(llvm::errs());
	if(vdecl->isLocalVarDecl()){
	  llvm::errs()<<"LocalDecl\n";
	}
	int dim = content->getNumInits()/2 -1;
	assert((content));
	{
	  clang::SourceRange SR = getPragmaSourceRange(vdecl);
	  std::string codestr;
	  llvm::raw_string_ostream ss(codestr);
	  clang::PrintingPolicy PP(ast.getLangOpts());
	  ss<<"/*Distribute*/\n";
	  initstream = &ss;
	  if(!vdecl->isLocalVarDecl()){
	    initstream = &epistream;
	  }

	  (*initstream)<<"/*\n";
	  (*initstream)<<"XMP_init_template_Chunk("<<TD->getName()<<","<<ND->getName() <<")\n";
	  for(int i = 0; i < dim; i++){
	    auto Mexpr = llvm::dyn_cast<clang::IntegerLiteral>
	      (content->getInit(i*2+2)->IgnoreCasts());
	    int64_t Meth = Mexpr->getValue().getSExtValue();
	    auto Pexpr = content->getInit(i*2+3)->IgnoreCasts();
	    
	    (*initstream)<<"XMP_dist_template_";
	    switch(Meth){
	    case 1:
	      (*initstream)<<"BLOCK";
	      break;
	    case 2:
	      (*initstream)<<"CYCLIC";
	      break;
	    case 3:
	      (*initstream)<<"WBLOCK";
	      break;
	    }
	    (*initstream)<<"(";
	    (*initstream)<<TD->getName();
	    (*initstream)<<","<<i;
	    (*initstream)<<",";
	    Pexpr->printPretty(*initstream, nullptr, PP);
	    (*initstream)<<");\n";
	  }
	  (*initstream)<<"*/\n";
	  rew.ReplaceText(SR,  ss.str().c_str());	  
	}

	return true;
}
