#include "Rewriter.h"

bool MyASTVisitor::DistributeHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
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
	  ss<<"/*\n";
	  ss<<"XMP_init_template_Chunk("<<TD->getName()<<","<<ND->getName() <<")\n";
	  for(int i = 0; i < dim; i++){
	    auto Mexpr = llvm::dyn_cast<clang::IntegerLiteral>
	      (content->getInit(i*2+2)->IgnoreCasts());
	    int64_t Meth = Mexpr->getValue().getSExtValue();
	    auto Pexpr = content->getInit(i*2+3)->IgnoreCasts();
	    
	    ss<<"XMP_dist_template_";
	    switch(Meth){
	    case 1:
	      ss<<"BLOCK";
	      break;
	    case 2:
	      ss<<"CYCLIC";
	      break;
	    case 3:
	      ss<<"WBLOCK";
	      break;
	    }
	    ss<<"(";
	    ss<<TD->getName();
	    ss<<","<<i;
	    ss<<",";
	    Pexpr->printPretty(ss, nullptr, PP);
	    ss<<");\n";
	  }
	  ss<<"*/\n";
	  rew.ReplaceText(SR,  ss.str().c_str());	  
	}

	return true;
}
