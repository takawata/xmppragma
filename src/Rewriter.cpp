#include "Rewriter.h"

/* まず、pragmaを置き換えた変数を見つける */
bool MyASTVisitor::VisitVarDecl(clang::VarDecl *vdecl){
    std::string name = vdecl->getName().str();
    
    if(name.find("__xmp_node") == 0){
      return NodeHandler(vdecl);
    }
    if(name.find("__xmp_align") == 0){
      return AlignHandler(vdecl);
    }

    llvm::errs()<<vdecl->getName()<<"\n";
    return true;
}
/* 関数宣言を見つけると呼ばれるコールバック関数 */
bool MyASTVisitor::VisitFunctionDecl(clang::FunctionDecl *fdecl) {
  rew.InsertTextBefore(fdecl->getBeginLoc(),
		       "/* found during AST traversal */\n");
  return true;
}

bool MyASTVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE)
{
  auto T = ASE->getType();
  if(T->isCompoundType()){
    return true;
  }
  clang::Expr *E;
  clang::DeclRefExpr *DRE;
  //std::vector<clang::Expr *> Idxs;
  
  for(clang::ArraySubscriptExpr *IT = ASE ; IT;){
    E = IT->getBase();
    auto ICE = llvm::dyn_cast<clang::ImplicitCastExpr>(E);
    if(!ICE){
      llvm::errs()<<"Invalid\n";
      return true;
    }
    E= ICE->getSubExpr();
    IT = llvm::dyn_cast<clang::ArraySubscriptExpr>(E);
  }
  if(!(DRE = llvm::dyn_cast<clang::DeclRefExpr>(E))){
    return true;
  }
  auto VD = llvm::dyn_cast<clang::VarDecl>(DRE->getDecl());
  if(VD){
    auto res = std::find(AlignedVars.begin(), AlignedVars.end(), VD);
    if(res == AlignedVars.end()){
      return true;
    }
  }else{
    DRE->getDecl()->dump();
    return true;
  }
  auto SL = ASE->getBeginLoc();
  auto EL = ASE->getEndLoc();
  auto &SM = rew.getSourceMgr();
  clang::SourceRange SR(SL, EL);
  std::string var = VD->getName();
  var = "*" + var;
  rew.ReplaceText(SR, var.c_str());
  return true;
}
