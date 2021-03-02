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
  dimstmt->dump();
  assert(avdecl);
  assert(tvdecl);
  auto TSI = avdecl->getTypeSourceInfo();
  auto AType = TSI->getType();

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
      declcodestr = "= (void*)" + Varname ;
    }
    if(AType->isPointerType()){
      llvm::errs()<<"AVDECL:"<<avdecl->getKind();
      AType.print(ss, PP);
      ss<<Varname<<";\n";
    }else{
      AType->dump();
      AType.print(ss, PP);
      ss<<"* "<<"_XMP_DATA_"<<Varname<<";\n";
    }
    ss<<"static void * _XMP_DESC_"<<Varname<<declcodestr<<";\n";
    (*initss)<<"/*Template "<<tvdecl->getName()<<" Dimension "<<dim<<"*/\n";
    (*initss)<<"/*Array subscripts:";
    for(auto &it: NumElems){
      (*initss)<<it<<",";
    }
    (*initss)<<"*/\n";
    rew.ReplaceText(SR,  ss.str().c_str());

  }
  return true;
}
