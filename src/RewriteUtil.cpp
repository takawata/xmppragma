#include "Rewriter.h"
std::string MyASTVisitor::getReductionType(clang::QualType T)
{
  using BTT = clang::BuiltinType;    
  
  if(T->isBuiltinType()){
    auto BT = llvm::dyn_cast<clang::BuiltinType>(T);
    switch(BT->getKind()){
    case BTT::Bool:
      return "502/*_XMP_N_TYPE_BOOL*/";
    case BTT::Char8:
    case BTT::Char_U:
    case BTT::UChar:
      return "503/*_XMP_N_TYPE_UNSIGNED_CHAR*/";
    case BTT::Char_S:
    case BTT::SChar:
      return "504/*_XMP_N_TYPE_CHAR*/";
    case BTT::Short:
      return "505/*_XMP_N_TYPE_SHORT*/";
    case BTT::UShort:
      return "506/*_XMP_N_TYPE_UNSIGNED_SHORT*/";
    case BTT::Int:
      return "507/*_XMP_N_TYPE_INT*/";
    case BTT::UInt:
      return "508/*_XMP_N_TYPE_UNSIGNED_INT*/";
    case BTT::Long:
      return "509/*_XMP_N_TYPE_LONG*/";
    case BTT::ULong:
      return "510/*_XMP_N_TYPE_UNSIGNED_LONG*/";
    case BTT::LongLong:
      return "511/*_XMP_N_TYPE_LONGLONG*/";
    case BTT::ULongLong:
      return "512/*_XMP_N_TYPE_UNSIGNED_LONGLONG*/";
    case BTT::Float:
      return "513/*_XMP_N_TYPE_FLOAT*/";
    case BTT::Double:
      return "514/*_XMP_N_TYPE_DOUBLE*/";
    case BTT::LongDouble:
      return "515/*_XMP_N_TYPE_LONG_DOUBLE*/";
    default:
      return "507/*_XMP_N_TYPE_INT*/";
    }
  }else if(T->isComplexType()){
    auto ET = llvm::dyn_cast<clang::ComplexType>(T)->getElementType();
    auto BT = llvm::dyn_cast<clang::BuiltinType>(T);
    if(!BT)
      return "507/*_XMP_N_TYPE_INT*/";
    switch(BT->getKind()){
    case BTT::Float:
      return "519/*_XMP_N_TYPE_FLOAT_COMPLEX*/";
    case BTT::Double:
      return "520/*_XMP_N_TYPE_DOUBLE_COMPLEX*/";
    case BTT::LongDouble:
      return "521/*_XMP_N_TYPE_LONG_DOUBLE_COMPLEX*/";
    default:
      return "507/*_XMP_N_TYPE_INT*/";
    }
  }
  return "507/*_XMP_N_TYPE_INT*/";
}
std::string MyASTVisitor::getReductionFunc(int reductionType)
{
	std::string reductionFunc;
	switch(reductionType){
	case clang::tok::plus:
		reductionFunc ="300/*_XMP_N_REDUCE_SUM*/";
		break;
	case clang::tok::star:
		reductionFunc ="301/*_XMP_N_REDUCE_PROD*/";
		break;
	case clang::tok::amp:
		reductionFunc ="302/*_XMP_N_REDUCE_BAND*/";
		break;
	case clang::tok::ampamp:
		reductionFunc ="303/*_XMP_N_REDUCE_LAND*/";
		break;
	case clang::tok::pipe:
		reductionFunc ="304/*_XMP_N_REDUCE_BOR*/";
		break;
	case clang::tok::pipepipe:
		reductionFunc ="305/*_XMP_N_REDUCE_LOR*/";
		break;
	case clang::tok::caret:
		reductionFunc ="306/*_XMP_N_REDUCE_XOR*/";
		break;
	case -1:
		reductionFunc ="307/*_XMP_N_REDUCE_MAX*/";
		break;
	case -2:
		reductionFunc ="308/*_XMP_N_REDUCE_MIN*/";
		break;
	case -3:
		reductionFunc ="309/*_XMP_N_REDUCE_FIRSTMAX*/";
		break;
	case -4:
		reductionFunc ="310/*_XMP_N_REDUCE_FIRSTMIN*/";
		break;
	case -5:
		reductionFunc ="311/*_XMP_N_REDUCE_LASTMAX*/";
		break;
	case -6:
		reductionFunc ="312/*_XMP_N_REDUCE_LASTMIN*/";
		break;
	}
	return reductionFunc;
}

clang::SourceRange MyASTVisitor::getPragmaSourceRange(clang::VarDecl *vdecl,
						      clang::Rewriter &r)
{
  
  clang::SourceLocation SL = vdecl->getBeginLoc();
  clang::SourceLocation EL = vdecl->getEndLoc();
  auto &SM = r.getSourceMgr();
  unsigned Line = SM.getSpellingLineNumber(SL);
  auto FID = SM.getFileID(SL);
  return clang::SourceRange(SM.translateLineCol(FID, Line, 1),
			    EL);
}
clang::SourceRange MyASTVisitor::getPragmaSourceRange(clang::VarDecl *vdecl)
{
  return getPragmaSourceRange(vdecl, rew);
}

clang::VarDecl *MyASTVisitor::getVarDeclFromDescArray(clang::InitListExpr *IL,
						      int element)
{
  auto node_stmt = llvm::dyn_cast<clang::UnaryOperator>
    (IL->getInit(element)->IgnoreCasts());
  auto declref = llvm::dyn_cast<clang::DeclRefExpr>
    (node_stmt->getSubExpr());
  auto node_decl = declref->getDecl();
  auto VD = llvm::dyn_cast<clang::VarDecl>(node_decl);
  
  return VD;
}
