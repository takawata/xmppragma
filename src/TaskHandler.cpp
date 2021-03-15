#include "Rewriter.h"

bool MyASTVisitor::TaskHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::SourceLocation SL;
	clang::VarDecl *taskvar;
	
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	taskvar = getVarDeclFromDescArray(content,0);
	
	{
	  clang::SourceRange SR = getPragmaSourceRange(vdecl);
	  std::string codestr;
	  llvm::raw_string_ostream ss(codestr);
	  ss<<"/*\n";
	  ss<<"*/\n";
	  rew.ReplaceText(SR,  ss.str().c_str());	  
	}

	return true;
}

bool MyASTVisitor::VisitIfStmt(clang::IfStmt *IST)
{
	auto Cond = IST->getCond();
	auto DRCond = llvm::dyn_cast<clang::DeclRefExpr>(Cond->IgnoreCasts());
	if(!DRCond)
		return true;
	auto VD = llvm::dyn_cast<clang::VarDecl>(DRCond->getDecl());
	std::string name = VD->getName().str();
	if(name.find("__xmp_task") != 0)
		return true;
	clang::PrintingPolicy PP(ast.getLangOpts());	
	static unsigned int descid = 0;
	std::string beforestr;
	std::string afterstr;
	llvm::raw_string_ostream bs(beforestr);
	llvm::raw_string_ostream as(afterstr);
	std::string taskdesc;
	auto content = llvm::dyn_cast<clang::InitListExpr>(VD->getInit());
	clang::VarDecl *taskvar;
	if(!content)
		return true;
	taskvar = getVarDeclFromDescArray(content,0);
	auto taskcont = llvm::dyn_cast<clang::InitListExpr>(taskvar->getInit());
	auto nodevar = getVarDeclFromDescArray(taskcont, 0);

	auto dimstmt = llvm::dyn_cast<clang::IntegerLiteral>
		(taskcont->getInit(1)->IgnoreCasts());
	int64_t dim = dimstmt->getValue().getSExtValue();
	taskdesc = "_XMP_TASK_desc" + std::to_string(descid);
	descid++;
	bs <<"{\n"<<"void *"<< taskdesc <<" = 0;\n";
	bs <<"if(_XMP_exec_task_NODES_PART(&"<<taskdesc;
	bs <<","<<nodevar->getName().str();
	for(int i = 0; i < dim; i++){
		bs<<", 0, ";
		clang::Expr *x,*first;
		clang::Expr::EvalResult ev;
		first = x = taskcont->getInit(2+3*i)->IgnoreCasts();
		
		if(x->EvaluateAsInt(ev,ast)
		   && ev.Val.getInt().getSExtValue()==-1){
			bs<< "-1,";
		}else{
			x->printPretty(bs, nullptr, PP);
			bs<<"+1,";
		}
		x= taskcont->getInit(3+3*i)->IgnoreCasts();
		if(x->EvaluateAsInt(ev, ast)
		   && ev.Val.getInt().getSExtValue() == -1){
			first->printPretty(bs, nullptr, PP);
			bs<<"+1,";
		}else{
			x->printPretty(bs, nullptr, PP);
			bs<<"+1,";
		}
		x= taskcont->getInit(4+3*i)->IgnoreCasts();
		if(x->EvaluateAsInt(ev, ast)
		   && ev.Val.getInt().getSExtValue() == -1){		   
			bs<<"1";
		}else{
			x->printPretty(bs, nullptr,PP);
		}
	}

	bs<<")){\n";
	as <<"\n_XMP_pop_nodes();\n}\n";
	as <<"_XMP_exec_task_NODES_FINALIZE("<<taskdesc<<");\n}\n";
	llvm::errs()<<"TASK FOUND\n";
	auto Body = IST->getThen();
	auto SR = Body->getSourceRange();
	auto B = SR.getBegin();
	auto E = SR.getEnd();
	rew.InsertTextBefore(B, bs.str());
	rew.InsertTextAfter(E.getLocWithOffset(2), as.str());	


	return true;
}
