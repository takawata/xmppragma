#include "Rewriter.h"

bool MyASTVisitor::NodeHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	
	llvm::errs()<<"NODE"<<"\n";
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	assert((content));
	auto node_stmt = llvm::dyn_cast<clang::UnaryOperator>
		(content->getInit(0)->IgnoreCasts());
	auto declref = llvm::dyn_cast<clang::DeclRefExpr>
		(node_stmt->getSubExpr());
	auto node_decl = declref->getDecl();
	node_decl->dump();
	auto VD = llvm::dyn_cast<clang::VarDecl>(node_decl);
	
	assert(VD);
	if(VD&&VD->isFunctionOrMethodVarDecl()){
		llvm::errs()<<"Local Decl\n";
	}
	VD->print(llvm::errs());
	std::string nname = VD->getName();
	int dim = content->getNumInits() - 1;
	
	llvm::errs()<<"Dimension"<< dim<<"\n";
	for(int i = 1; i < content->getNumInits(); i++){
		if( content->getInit(i)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
			llvm::errs()<<ev.Val.getInt()<<"\n";
		}else{
			llvm::errs()<<"ConvErr"<<"\n";
		}
	}
	{
	  SL = vdecl->getBeginLoc();
	  clang::SourceLocation EL = vdecl->getEndLoc();
	  auto &SM = rew.getSourceMgr();
	  std::string codestr;
	  llvm::raw_string_ostream ss(codestr);
	  unsigned Line = SM.getSpellingLineNumber(SL);
	  auto FID = SM.getFileID(SL);
	  clang::SourceRange SR(SM.translateLineCol(FID, Line, 1),
				EL);
	  VD->print(ss);
	  ss<<";\n";
	  for(int i = 1; i < dim; i++){
		  ss<<"static int __XMP_NODES_SIZE_"<<nname<<i<<";\n";
	  }
	  
	  for(int i = 0; i < dim; i++){
		  ss<<"static int __XMP_NODES_RANK_"<<nname<<i<<";\n";
	  }
	  ss<<"/*";
	  ss<<"XMP_init_nodes_DYNAMIC_GLOBAL(&"<<nname<<","<< dim <<");";
	  ss<<"*/"<<"\n";
	  rew.ReplaceText(SR,  ss.str().c_str());
	}
	return true;
}
