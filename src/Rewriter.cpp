#include "Rewriter.h"
#include <clang/AST/Expr.h>
#include <filesystem>

MyASTVisitor::MyASTVisitor(clang::Rewriter &r,clang::ASTContext &a) : rew(r),ast(a),epistream(epiloge) {
  auto MFID = rew.getSourceMgr().getMainFileID();
  auto SFLOC = rew.getSourceMgr().getLocForStartOfFile(MFID);
  auto FILENAME = rew.getSourceMgr().getFilename(SFLOC);
  std::__fs::filesystem::path p(FILENAME);
  epistream <<"int main(int argc, char *argv[])\n";
  epistream <<"{\n xmp_init_all(argc, argv);\n xmpc_main(argc,argv);\n";
  epistream <<"xmp_finalize_all();}\n";  
  epistream<<"extern void  xmpc_traverse_init_file_"<<p.stem()<<"() {\n";
}

/* まず、pragmaを置き換えた変数を見つける */
bool MyASTVisitor::VisitVarDecl(clang::VarDecl *vdecl){
    std::string name = vdecl->getName().str();
    
    if(name.find("__xmp_node") == 0){
      return NodeHandler(vdecl);
    }
    if(name.find("__xmp_align") == 0){
      return AlignHandler(vdecl);
    }
    if(name.find("__xmp_template") == 0){
      return TemplateHandler(vdecl);
    }
    if(name.find("__xmp_loop") == 0){
      return LoopHandler(vdecl);
    }
    if(name.find("__xmp_distribute") == 0){
      return DistributeHandler(vdecl);
    }
    if(name.find("__xmp_reflect") == 0){
      return ReflectHandler(vdecl);
    }
    if(name.find("__xmp_shadow") == 0){
      return ShadowHandler(vdecl);
    }
    if(name.find("__xmp_task") == 0){
      return TaskHandler(vdecl);
    }
    if(name.find("__xmp_bcast") == 0){
      return BcastHandler(vdecl);
    }

    llvm::errs()<<vdecl->getName()<<"\n";
    return true;
}
/* 関数宣言を見つけると呼ばれるコールバック関数 */
bool MyASTVisitor::VisitFunctionDecl(clang::FunctionDecl *fdecl) {
  if((fdecl->getName().str()=="main")&&fdecl->hasBody()){
    auto DN = fdecl->getNameInfo();
    
    rew.ReplaceText(DN.getSourceRange() , "xmpc_main");
  }
  return true;
}

bool MyASTVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE)
{
  std::vector<clang::Expr*> IdxList;
  auto T = ASE->getType();
  if(T->isArrayType()||T->isPointerType()){
    return true;
  }
  clang::Expr *E;
  clang::DeclRefExpr *DRE;
  std::string codestr;
  llvm::raw_string_ostream ss(codestr);
  
  for(clang::ArraySubscriptExpr *IT = ASE ; IT;){
    E = IT->getBase();
    clang::Expr* IdE = IT->getIdx();
    IdxList.push_back(IdE);
    auto ICE = llvm::dyn_cast<clang::ImplicitCastExpr>(E);
    if(!ICE){

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
  ss <<"(*(_XMP_DATA_" << var;
  ss<<"+(";
  {
    int i = 0;
    clang::PrintingPolicy PP(ast.getLangOpts());

    for(auto IT = IdxList.begin();;){
      (*IT)->printPretty(ss, nullptr, PP);
      IT++;
      if(IT == IdxList.end())
	break;
      ss<<")+_XMP_GTOL_acc_"<<var<<"_"<<i<<"*(";
      i++;
    }
    ss<<")))";
  }
  rew.ReplaceText(SR, ss.str().c_str());
  return true;
}

bool MyASTVisitor::VisitCallExpr(clang::CallExpr *CE)
{
  auto Args = CE->getArgs();
  for(int i = 0; i < CE->getNumArgs(); i++ ){
    auto Arg = llvm::dyn_cast<clang::DeclRefExpr>(Args[i]->IgnoreCasts());
    if(Arg == nullptr){
      continue;
    }
    auto VD = llvm::dyn_cast<clang::VarDecl>(Arg->getDecl());

    if(VD){
      auto res = std::find(AlignedVars.begin(), AlignedVars.end(), VD);
      if(res == AlignedVars.end()){
	continue;
      }
    }
    clang::SourceRange ASR(Arg->getBeginLoc(), Arg->getEndLoc());
    std::string varname = VD->getName();
    rew.ReplaceText(ASR, "_XMP_DATA_"+varname);
  }
  return true;
}
