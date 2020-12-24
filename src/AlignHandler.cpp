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
  /*Process Aligned variable*/
  AlignedVars.push_back(avdecl);
  std::string Varname = avdecl->getName();
  std::vector <int> NumElems;
  {
    auto AType = avdecl->getType();
    std::string codestr;
    llvm::raw_string_ostream ss(codestr);
    clang::PrintingPolicy PP(ast.getLangOpts());

    while(1){
      auto caType = llvm::dyn_cast<clang::ConstantArrayType>(AType);
      if(caType == nullptr)
	break;
      AType = caType->getElementType();
      llvm::errs()<<caType->getSize().getZExtValue()<<"\n";
      NumElems.push_back(caType->getSize().getZExtValue());
    }
    clang::SourceRange SR(avdecl->getBeginLoc(), avdecl->getEndLoc());
    if(AType->isPointerType()){
      NumElems.push_back(-1);
      AType.print(ss, PP);
      ss<<Varname;
    }else{
      AType->dump();
      AType.print(ss, PP);
      ss<<"* "<<Varname;
    }
    rew.ReplaceText(SR,  ss.str().c_str());
  }
  /*Process descriptor*/
  {
    std::string codestr;
    llvm::raw_string_ostream ss(codestr);
    clang::SourceRange SR = getPragmaSourceRange(vdecl);

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
