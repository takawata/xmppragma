#include "Rewriter.h"

LoopDesc::LoopDesc(clang::VarDecl *pdecl, clang::VarDecl *node, int dim, clang::Rewriter &r):PragmaDecl(pdecl),NodeDecl(node),LoopCounts(dim),InitValues(dim),Loops(dim),rew(r),ReductionDecl(nullptr){
};

LoopDesc::LoopDesc(clang::VarDecl *pdecl, clang::VarDecl *node, int dim, clang::Rewriter &r,int reductionType,clang::VarDecl *rdecl):PragmaDecl(pdecl),NodeDecl(node),LoopCounts(dim),InitValues(dim),Loops(dim),rew(r),ReductionDecl(rdecl)
{
	reductionFunc = MyASTVisitor::getReductionFunc(reductionType);
};

std::string LoopDesc::getReductionInit()
	{
		if(!ReductionDecl){
			return std::string("");
		}
		std::string rvName = ReductionDecl->getName();		
		return std::string(
			"if((_XMP_get_execution_nodes_rank()) != 0)\n") +
			rvName + "= 0;\n";
	}
std::string LoopDesc::getReduction(){
		if(!ReductionDecl){
			return std::string("");
		}
		std::string rvName = ReductionDecl->getName();
		clang::QualType rvType = ReductionDecl->getType();
		std::string typeName = MyASTVisitor::getReductionType(rvType);
		return  std::string("_XMP_reduce_CLAUSE(") + "&"+ rvName+","+"1,"+typeName+","+reductionFunc+");";
}
void LoopDesc::setLoopCount(int x,int y, int pos)
{
	int dim = LoopCounts.size();
	assert(pos < dim);
	LoopCounts[pos] = x;
	InitValues[pos] = y;
	Loops --;
	if(Loops == 0){
		auto content =
			llvm::dyn_cast<clang::InitListExpr>
			(PragmaDecl->getInit());
		std::string codestr;
		llvm::raw_string_ostream ss(codestr);
		clang::SourceRange SR =
			MyASTVisitor::getPragmaSourceRange
			(PragmaDecl, rew);
		ss<<getReductionInit();
		ss<<"/*Pragma Loop*/\n{\n";
		NodeDecl->print(ss);
		ss<<";\n";
		ss<<"/*Node :"<<NodeDecl->getName()<<"*/\n";
		
		for(int i=0; i < dim ; i++){
			auto loopVar = 
				MyASTVisitor::getVarDeclFromDescArray(
					content , i + 2);
			std::string lvname = loopVar->getName();
			std::string initname = "_XMP_loop_init_"+lvname;
			std::string stepname = "_XMP_loop_step_"+lvname;
			std::string countname = "_XMP_loop_cond_"+lvname;
			ss<<"int "<< lvname <<";\n";
			ss<<"int "<<initname<<"= "<<InitValues[i]<<";\n";
			ss<<"int "<<stepname<<"= 1;\n";
			ss<<"int "<<countname<<"= "<<LoopCounts[i]<<";\n";
			ss<<"xmpc_loop_sched("<<
				initname<<","<<
				countname<<","<<
				stepname<<","<<
				"&"<<initname<<","<<
				"&"<<countname<<","<<
				"&"<<stepname<<", "<<"*(void**)"<<
				NodeDecl->getName()<<"[0],"<<
				dim-i-1<<",413/* _XMP_LOOP_NONE*/,0,0,0"<<
				");\n";
		}
		rew.ReplaceText(SR, ss.str().c_str());
	}
}

bool MyASTVisitor::LoopHandler(clang::VarDecl *vdecl)
{
	vdecl->print(llvm::errs());
	clang::Expr::EvalResult ev;
	clang::DeclContext *DC = vdecl->getLexicalDeclContext();
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	assert((content));
	llvm::errs()<<"LOOP"<<"\n";
	auto VD = getVarDeclFromDescArray(content, 0);
	

	assert(VD);
	if(VD&&VD->isFunctionOrMethodVarDecl()){
		llvm::errs()<<"Local Decl\n";
	}
	VD->print(llvm::errs());
	std::string nname = VD->getName();
	auto dimstmt = llvm::dyn_cast<clang::IntegerLiteral>(content->getInit(1)
							     ->IgnoreCasts());
	int64_t dim = dimstmt->getValue().getSExtValue();
	std::shared_ptr<LoopDesc> LD;
	if(content->getNumInits() ==  dim+4){
		clang::Expr::EvalResult ev;
		auto redFuncExpr = content->getInit(dim+2)->IgnoreCasts();
		int redfunc = 0;
		auto redVar = getVarDeclFromDescArray(content, dim+3);
		if(redFuncExpr->EvaluateAsInt(ev, ast)){
			redfunc = ev.Val.getInt().getSExtValue();
		}
		llvm::errs()<<"Reduction"<<redfunc<<"\n";
		LD = std::make_shared<LoopDesc>(vdecl, VD, dim, rew, redfunc, redVar);
	}else{
		LD = std::make_shared<LoopDesc>(vdecl, VD, dim, rew);
	}
	for(int i = 0; i < dim; i++){
	  auto loopVar = getVarDeclFromDescArray(content, 2+i);
	  Loops.push_back(LoopInfo(loopVar, LD, i));
	}
	return true;
}

bool MyASTVisitor::TraverseForStmt(clang::ForStmt *FST)
{
	auto CondStmt =  FST->getCond();
	auto CondVOp = llvm::dyn_cast<clang::BinaryOperator>(CondStmt);

	auto VRef = llvm::dyn_cast<clang::DeclRefExpr>(CondVOp->getLHS()->IgnoreCasts());
	int ret;

	clang::Expr::EvalResult ev;
	if(!VRef)
		return true;

	auto V = llvm::dyn_cast<clang::VarDecl>(VRef->getDecl());
	if(!V)
		return BASE::TraverseForStmt(FST);
	ret = WalkUpFromForStmt(FST);
	if(!ret)
		return ret;

	ret = TraverseStmt(FST->getBody());

	return ret;
}

bool MyASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr *DRE)
{
	auto V = DRE->getDecl();
	auto LI = std::find_if(Loops.begin(), Loops.end(),
			      [V](LoopInfo &it)
			      { return it.loopVar == V;});
	if(LI == Loops.end())
		return true;

	std::string codestr;
	llvm::raw_string_ostream ss(codestr);
	auto LV = LI->loopVar;
	auto LD = LI->LD;
	auto ND = LD->NodeDecl;
	auto Content = llvm::dyn_cast<clang::InitListExpr>(ND->getInit());
	auto TD = getVarDeclFromDescArray(Content, 0);
	auto DI = std::find_if(Dists.begin(), Dists.end(),
			       [TD, LI](auto X){
				       return ((X.tempdecl == TD)
					       && (X.pos == LI->Order));
			       });
	if(DI == Dists.end()){
		llvm::errs()<<"DIST NOT FOUND"<<TD->getName()<<","
			    <<LI->Order<<"\n";
		exit(-1);
	}
	auto TV = std::find_if(TempVars.begin(), TempVars.end(),
			       [TD](auto X) {return (X.first == TD);});
	if(TV == TempVars.end()){
		llvm::errs()<<"TEMP NOT FOUND";
		exit(-1);
	}
	auto TI = llvm::dyn_cast<clang::InitListExpr>(TV->second->getInit());
	auto dim = TI->getNumInits()-1;
	auto order = dim - 1 - LI->Order;
	int tsize;
	clang::Expr::EvalResult ev;
	if(TI->getInit(order+1)->IgnoreCasts()->EvaluateAsInt(ev,ast)){
	  tsize = ev.Val.getInt().getSExtValue();
	}
	ss<<"_XMP_M_LTOG_TEMPLATE_"<<DI->type<<"("<<V->getName()<<",";
	ss<<"0,"<<tsize<<","<<"__XMP_NODES_SIZE_"<<DI->nodedecl->getName()<<order<<",";
	ss<<"__XMP_NODES_RANK_"<<DI->nodedecl->getName()<<order<<")";
	rew.ReplaceText(DRE->getSourceRange(), ss.str());
	V->getSourceRange().dump(rew.getSourceMgr());

	return true;
}

bool MyASTVisitor::VisitForStmt(clang::ForStmt *FST)
{
	auto CondStmt =  FST->getCond();
	auto CondVOp = llvm::dyn_cast<clang::BinaryOperator>(CondStmt);
	
	auto VRef = llvm::dyn_cast<clang::DeclRefExpr>(CondVOp->getLHS()->IgnoreCasts());
	clang::Expr::EvalResult ev;
	if(!VRef)
		return true;
	
	auto V = llvm::dyn_cast<clang::VarDecl>(VRef->getDecl());
	if(!V)
		return true;
	auto r = std::find_if(Loops.begin(), Loops.end(),
			      [V](LoopInfo &it)
			      { return it.loopVar == V;});
	if(r == Loops.end()){
		return true;
	} else{
		clang::Expr *CountExpr = CondVOp->getRHS();
		auto *InitStmt = llvm::dyn_cast<clang::BinaryOperator>
			(FST->getInit());
		clang::Expr *InitVExpr = InitStmt->getRHS();
		clang::Expr *IncExpr = FST->getInc();
		int64_t Count = 0;
		int64_t Init = 0;
		int64_t Step = 0;
		if(CountExpr->EvaluateAsInt(ev, ast)){
			Count = ev.Val.getInt().getSExtValue();
		}
		if(InitVExpr->EvaluateAsInt(ev, ast)){
			Init = ev.Val.getInt().getSExtValue();
		}
		
		auto UOP = llvm::dyn_cast<clang::UnaryOperator>(IncExpr);
		if(UOP){
			switch(UOP->getOpcode()){
			case clang::UO_PostInc:
			case clang::UO_PreInc:
				Step = 1;
				break;
			case clang::UO_PostDec:
			case clang::UO_PreDec:
				Step = -1;
				break;
			default:
				llvm::errs()<<"Unknown Operator\n";
			}
		}else {
			auto BOP = llvm::dyn_cast<clang::BinaryOperator>(IncExpr);
			if(BOP){
				Step = 1;
				switch(BOP->getOpcode()){
				case clang::BO_AddAssign:
					if(BOP->getRHS()->EvaluateAsInt(ev, ast)){
						Step = ev.Val.getInt().getSExtValue();
					}
					break;
				case clang::BO_SubAssign:
					if(BOP->getRHS()->EvaluateAsInt(ev, ast)){
						Step = -ev.Val.getInt().getSExtValue();
					}
					break;
				default:
					llvm::errs()<<"Unknown Operators\n";
				}
			}else{
				llvm::errs()<<"Unknown Expr\n";
			}
		}
		
		r->LD->setLoopCount(Count, Init, r->Order);
		std::string loopVarName = V->getName();
		{
			auto SL= FST->getLParenLoc();
			auto EL = FST->getRParenLoc();
			std::string codestr;
			llvm::raw_string_ostream ss(codestr);
			clang::SourceRange SR(SL,EL);
			ss<<"("<<loopVarName<<" = " << "_XMP_loop_init_"
			  <<loopVarName<<";";
			ss<<loopVarName <<" <= " << "_XMP_loop_cond_"<<loopVarName <<";";
			ss<<loopVarName<< " += " << "_XMP_loop_step_"<< loopVarName<<")";

			rew.ReplaceText(SR, ss.str().c_str());
		}

		if(r->Order == 0){
			auto ED = FST->getEndLoc();
			clang::Rewriter::RewriteOptions rangeOpts;
			rangeOpts.IncludeInsertsAtBeginOfRange = false;
			auto  sz = rew.getRangeSize(clang::SourceRange(ED, ED),
						    rangeOpts) + 1;
			auto endblock = std::string("\n") +
				r->LD->getReduction() + std::string("}\n");
			rew.InsertText(ED.getLocWithOffset(sz),
				       endblock.c_str());
		}
		llvm::errs()<<"FIND_LOOP "<<Init<<","<<Count<<","<<Step<<"\n";
		
		V->dump();
	}
	return true;
}

