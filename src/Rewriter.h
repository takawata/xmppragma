#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>
#include <iostream>
class MyASTVisitor;
class LoopDesc{
	std::vector <int> LoopCounts;
	std::vector <int> InitValues;
	clang::VarDecl *PragmaDecl;
	clang::VarDecl *NodeDecl;
	clang::VarDecl *ReductionDecl;
	std::string reductionFunc;
	int Loops;
	clang::Rewriter& rew;
public:
	LoopDesc(clang::VarDecl *pdecl, clang::VarDecl *node, int dim, clang::Rewriter &r);	
	LoopDesc(clang::VarDecl *pdecl, clang::VarDecl *node, int dim, clang::Rewriter &r,int reductionType,clang::VarDecl *rdecl);
	std::string getReductionInit();
	std::string getReduction();
	void setLoopCount(int x,int y, int pos);
};

/* AST traversal */
class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
  clang::Rewriter &rew;
  std::string epiloge;
  llvm::raw_string_ostream epistream;
  struct LoopInfo{
    clang::VarDecl *loopVar;
    std::shared_ptr<LoopDesc> LD;
    int Order;
    LoopInfo(clang::VarDecl *vd,std::shared_ptr<LoopDesc> ld, int f):loopVar(vd),LD(ld),Order(f)
    {}
  };
  std::vector<LoopInfo> Loops;
  struct AllocInfo{
	  clang::VarDecl *aligndecl;
	  clang::VarDecl *pragmadecl;
	  int64_t dim;
  };
  std::vector<AllocInfo> Allocs;
  struct DistInfo{
	  clang::VarDecl *tempdecl;
	  int pos;
	  const char *type;
  };
  bool hasmain;
  std::vector<DistInfo> Dists;
  clang::ASTContext &ast;
  std::vector<clang::VarDecl*> AlignedVars;
  std::vector<std::pair<clang::VarDecl*, clang::VarDecl*> > ShadowVars;
  bool NodeHandler(clang::VarDecl *vdecl);
  bool BcastHandler(clang::VarDecl *vdecl);
  bool AlignHandler(clang::VarDecl *vdecl);
  bool TemplateHandler(clang::VarDecl *vdecl);
  bool LoopHandler(clang::VarDecl *vdecl);
  bool DistributeHandler(clang::VarDecl *vdecl);
  bool ReflectHandler(clang::VarDecl *vdecl);
  bool ShadowHandler(clang::VarDecl *vdecl);
  bool TaskHandler(clang::VarDecl *vdecl);
  clang::SourceRange getPragmaSourceRange(clang::VarDecl *vdecl); 
public:
  static clang::SourceRange getPragmaSourceRange(clang::VarDecl *vdecl, clang::Rewriter &r);
  static clang::VarDecl *getVarDeclFromDescArray(clang::InitListExpr *, int);
  static std::string getAllocString(AllocInfo &);
  static std::string getReductionFunc(int x);
  static std::string getReductionType(clang::QualType T);
  MyASTVisitor(clang::Rewriter &r,clang::ASTContext &a);
  std::string &getEpiloge();
  void AddAllocFuncAtLast();
  /* まず、pragmaを置き換えた変数を見つける */
  bool VisitVarDecl(clang::VarDecl *vdecl);
  bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr *ASE);
  /* 関数宣言を見つけると呼ばれるコールバック関数 */
  bool VisitFunctionDecl(clang::FunctionDecl *fdecl);
  bool VisitForStmt(clang::ForStmt *FST);
  bool VisitCallExpr(clang::CallExpr *CST);
  bool VisitIfStmt(clang::IfStmt *IST);
};
