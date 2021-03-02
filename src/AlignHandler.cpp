#include "Rewriter.h"

bool MyASTVisitor::AlignHandler(clang::VarDecl *vdecl)
{
  auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
  assert((content));
  auto avdecl = getVarDeclFromDescArray(content, 0);
  auto tvdecl = getVarDeclFromDescArray(content, 1);
  auto dimstmt = llvm::dyn_cast<clang::IntegerLiteral>(content->getInit(2)
						       ->IgnoreCasts());
  int64_t dim = dimstmt->getValue().getSExtValue();
  dimstmt->dump();
  assert(avdecl);
  assert(tvdecl);
  auto AType = avdecl->getType();

  /*Process Aligned variable*/
  AlignedVars.push_back(avdecl);
  std::string Varname = avdecl->getName();
  std::vector <int> NumElems;
  {
    while(1){
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
    llvm::raw_string_ostream dss(declcodestr);
    llvm::raw_string_ostream *declss = &ss;
    clang::SourceRange SR = getPragmaSourceRange(vdecl);
    clang::PrintingPolicy PP(ast.getLangOpts());
    clang::SourceRange ASR(avdecl->getBeginLoc(), avdecl->getEndLoc());
    if(avdecl->getKind() != clang::Decl::ParmVar){
      rew.ReplaceText(ASR, "/*original vardecl*/");
    }else{
      declss = &dss;
    }

    if(AType->isPointerType()&&!(avdecl->getKind() != clang::Decl::ParmVar)){
      AType.print(ss, PP);
      (*declss)<<Varname<<";\n/*X*/";
    }else{
      AType->dump();
      AType.print(ss, PP);
      (*declss)<<"* "<<Varname<<";/*Y*/\n";
    }
    ss<<"static void * _XMP_DESC_"<<Varname<<";\n";
    ss<<"/*Template "<<tvdecl->getName()<<" Dimension "<<dim<<"*/\n";
    ss<<"/*Array subscripts:";
    for(auto &it: NumElems){
      ss<<it<<",";
    }
    ss<<"*/\n";
    rew.ReplaceText(SR,  ss.str().c_str());
  }
  return true;
}
