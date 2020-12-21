#include "Rewriter.h"

/* まず、pragmaを置き換えた変数を見つける */
bool MyASTVisitor::VisitVarDecl(clang::VarDecl *vdecl){
    std::string name = vdecl->getName().str();
    
    if(name.find("__xmp_node") == 0){
      return NodeHandler(vdecl);
    }
    if(name.find("__xmp_align") == 0){
      auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
      assert((content));
      auto align_stmt = llvm::dyn_cast<clang::UnaryOperator>
	(content->getInit(0)->IgnoreCasts());
      auto declref = llvm::dyn_cast<clang::DeclRefExpr>
	(align_stmt->getSubExpr());
      auto align_decl = declref->getDecl();
      align_decl->dump();
      rew.InsertTextBefore(vdecl->getBeginLoc(),
			   "/* Pragma Align Found */");
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
