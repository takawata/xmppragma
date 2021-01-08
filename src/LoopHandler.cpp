#include "Rewriter.h"
class LoopDesc{
	std::vector <int> LoopCounts;
	clang::VarDecl *PragmaDecl;
	clang::VarDecl *NodeDecl;

	int Loops;
	clang::Rewriter& rew;
public:
	LoopDesc(clang::VarDecl *pdecl, clang::VarDecl *node, int dim, clang::Rewriter &r):PragmaDecl(pdecl),NodeDecl(node),LoopCounts(dim),Loops(dim),rew(r){};
	
	std::string getReduction(){
		return std::string("/*Reduction*/");
	}
	void setLoopCount(int x, int pos)
	{
		int dim = LoopCounts.size();
		assert(pos < dim);
		LoopCounts[pos] = x;
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

			ss<<"/*Pragma Loop*/\n{\n";
			ss<<"/*Node :"<<NodeDecl->getName()<<"*/\n";
			for(int i=0; i < dim ; i++){
				auto loopVar = 
					MyASTVisitor::getVarDeclFromDescArray(
						content , i + 2);
				std::string lvname = loopVar->getName();
				ss<<"int "<< lvname <<";\n";
				ss<<"int _xmp_init_"<<lvname<<"= 0;\n";
				ss<<"int _xmp_step_"<<lvname<<"= 1;\n";
				ss<<"int _xmp_count"<<lvname<<"= "<<LoopCounts[i]<<";\n";
			}
			rew.ReplaceText(SR, ss.str().c_str());
		}
	}
	
};
bool MyASTVisitor::LoopHandler(clang::VarDecl *vdecl)
{
	clang::Expr::EvalResult ev;
	clang::DeclContext *DC = vdecl->getLexicalDeclContext();
	auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
	assert((content));
	llvm::errs()<<"LOOP"<<"\n";
	auto VD = getVarDeclFromDescArray(content, 0);
	
	vdecl->print(llvm::errs());
	assert(VD);
	if(VD&&VD->isFunctionOrMethodVarDecl()){
		llvm::errs()<<"Local Decl\n";
	}
	VD->print(llvm::errs());
	std::string nname = VD->getName();
	auto dimstmt = llvm::dyn_cast<clang::IntegerLiteral>(content->getInit(1)
							     ->IgnoreCasts());
	int64_t dim = dimstmt->getValue().getSExtValue();
	auto LD = new LoopDesc(vdecl, VD, dim, rew);
	for(int i = 0; i < dim; i++){
	  auto loopVar = getVarDeclFromDescArray(content, 2+i);
	  Loops.push_back(LoopInfo(loopVar, LD, i));
	}
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
		
		r->LD->setLoopCount(Count, r->Order);
		std::string loopVarName = V->getName();
		{
			auto SL= FST->getLParenLoc();
			auto EL = FST->getRParenLoc();
			std::string codestr;
			llvm::raw_string_ostream ss(codestr);
			clang::SourceRange SR(SL,EL);			
			ss<<"("<<loopVarName<<" = " << "_xmp_init_"
			  <<loopVarName<<";";
			ss<<loopVarName <<" <= " << "_xmp_count_"<<loopVarName <<";";
			ss<<loopVarName<< " += " << "_xmp_step_"<< loopVarName<<")";

			rew.ReplaceText(SR, ss.str().c_str());
		}

		if(r->Order == 0){
			auto ED = FST->getEndLoc();
			auto endblock = std::string("}\n") +
				r->LD->getReduction() + std::string("\n");
			rew.InsertTextAfter(ED, endblock.c_str());
		}
		llvm::errs()<<"FIND_LOOP "<<Init<<","<<Count<<","<<Step<<"\n";
		
		V->dump();
	}
	return true;
}

