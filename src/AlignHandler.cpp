#include "Rewriter.h"

bool MyASTVisitor::AlignHandler(clang::VarDecl *vdecl)
{
  auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
  assert((content));
  auto avdecl = getVarDeclFromDescArray(content, 0);
  auto tvdecl = getVarDeclFromDescArray(content, 1);
  auto dimstmt = llvm::dyn_cast<clang::IntegerLiteral>(content->getInit(2)
						       ->IgnoreCasts());
  clang::PrintingPolicy PP(ast.getLangOpts());
  int64_t dim = dimstmt->getValue().getSExtValue();
  bool needalloc = true;
  dimstmt->dump();
  assert(avdecl);
  assert(tvdecl);
  auto TSI = avdecl->getTypeSourceInfo();
  auto AType = TSI->getType();
  clang::QualType EType;
  
  /*Process Aligned variable*/
  AlignedVars.push_back(avdecl);
  std::string Varname = avdecl->getName();
  std::vector <int> NumElems;
  {
    while(1){
      AType.print(llvm::errs(), PP);
      auto caType = llvm::dyn_cast<clang::ConstantArrayType>(AType);
      if(caType == nullptr)
        break;
      AType = caType->getElementType();
      llvm::errs()<<caType->getSize().getZExtValue()<<"\n";
      NumElems.push_back(caType->getSize().getZExtValue());
    }
  }
  /*Process descriptor*/
  {
    std::string codestr;
    std::string declcodestr;
    llvm::raw_string_ostream ss(codestr);
    clang::SourceRange SR = getPragmaSourceRange(vdecl);
    clang::SourceRange ASR(avdecl->getBeginLoc(), avdecl->getEndLoc());
    llvm::raw_string_ostream *initss = &ss;
    if(!vdecl->isFunctionOrMethodVarDecl()){
      initss = &epistream;
    }
    
    if(avdecl->getKind() != clang::Decl::ParmVar){
      rew.ReplaceText(ASR, "/*original vardecl*/");
    }else{
      needalloc = false;
      declcodestr = "= (void*)" + Varname ;
    }
    std::string Dataname = "_XMP_DATA_"+Varname;
    std::string Descname = "_XMP_DESC_"+Varname;
    if(AType->isPointerType()){
      AType.print(ss, PP);
      AType = AType->getPointeeType();
      needalloc = false;
      dim++;
      ss<<Dataname<<";\n";
    }else{
      AType.print(ss, PP);
      ss<<"* "<<Dataname<<declcodestr<<";\n";
    }
    ss<<"static void *"<<Descname<<";\n";
    for(int i = 0; i < dim; i++){
      ss << "static unsigned long long  _XMP_GTOL_acc_"<< Varname<<"_"<<std::to_string(i)<<";\n";
      ss << "static int _XMP_GTOL_temp0_"<< Varname<<"_"<<std::to_string(i)<<";\n";
    }
    (*initss)<<"_XMP_init_array_desc(&"<<Descname<<","<<tvdecl->getName()<<","<<dim<<","<<getReductionType(AType)<<", sizeof(*"<<Dataname<<")";
    for(auto &it: NumElems){
      (*initss)<<","<<it;
    }
    (*initss)<<");\n";
    for(int i = 0 ; i < dim ; i++){
      auto DI = std::find_if(Dists.begin(), Dists.end(),
			     [tvdecl, i](auto X){return ((X.tempdecl == tvdecl)&&(X.pos == i));});
      (*initss)<<"_XMP_align_array_"<<DI->type<<"("<<Descname<<",";
      (*initss)<<std::to_string(i)<<","<<std::to_string(dim-i-1)<<", 0,";
      (*initss)<<"&_XMP_GTOL_temp0_"<<Varname<<"_"<<std::to_string(i)<<");\n";
    }
    (*initss)<<"_XMP_init_array_comm("<<Descname<<",0 ,0);\n";    
    (*initss)<<"_XMP_init_array_nodes("<<Descname<<");\n";
    if(needalloc){
      Allocs.push_back({avdecl, vdecl, dim});
    }
    rew.ReplaceText(SR,  ss.str().c_str());

  }
  return true;
}
void MyASTVisitor::AddAllocFuncAtLast()
{
  std::string str;
  llvm::raw_string_ostream ss (str);
  llvm::raw_string_ostream *initss;

  for(auto &Ai : Allocs) {
    bool needreplace = true;
    auto endloc = Ai.pragmadecl->getEndLoc();
    initss = &ss;
    if(!Ai.pragmadecl->isFunctionOrMethodVarDecl()){
      initss = &epistream;
      needreplace = false;
    }
    (*initss)<<getAllocString(Ai);
    if(needreplace) {
      rew.InsertTextAfter(endloc, ss.str());
    }
  }
  
}
std::string MyASTVisitor::getAllocString(AllocInfo &Ai)
{
  std::string name = Ai.aligndecl->getName();
  std::string str("_XMP_alloc_array((void**)&_XMP_DATA_");
  str = str +  name + "," + "_XMP_DESC_" + name+ ", 1/*iscoarray*/";

  for(int i = Ai.dim -1 ; i >= 0; i--){
    str = str + ", &_XMP_GTOL_acc_" + name + "_" + std::to_string(i);
  }
  str += ");\n";
  
  return str;
}
