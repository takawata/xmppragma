#include "clang/Basic/Version.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "PPUtil.h"
#include "PPclass.h"


/* ASTを受取るクラス */
class MyASTConsumer : public clang::ASTConsumer {
  clang::CompilerInstance *cmpi;
public:
  MyASTConsumer(clang::CompilerInstance *ci) : cmpi(ci) {
    auto &PP = ci->getPreprocessor();
    PP.AddPragmaHandler("xmp", new PragmaNodesHandler());
    PP.AddPragmaHandler("xmp", new PragmaTemplateHandler());
    PP.AddPragmaHandler("xmp", new PragmaDistributeHandler());
    PP.AddPragmaHandler("xmp", new PragmaAlignHandler());
    PP.AddPragmaHandler("xmp", new PragmaArrayHandler());
    PP.AddPragmaHandler("xmp", new PragmaLoopHandler());
    PP.AddPragmaHandler("xmp", new PragmaShadowHandler());
    PP.AddPragmaHandler("xmp", new PragmaReflectHandler());
    PP.AddPragmaHandler("xmp", new PragmaTaskHandler());
    PP.AddPragmaHandler("xmp", new PragmaTasksHandler());
    PP.AddPragmaHandler("xmp", new PragmaGmoveHandler());
    PP.AddPragmaHandler("xmp", new PragmaBcastHandler());
    PP.AddPragmaHandler("xmp", new PragmaWaitAsyncHandler());
    PP.AddPragmaHandler("xmp", new PragmaReductionHandler());
    PP.AddPragmaHandler("xmp", new PragmaBarrierHandler());
    PP.AddPragmaHandler("xmp", new PragmaReduceShadowHandler());
    PP.AddPragmaHandler("xmp", new PragmaTemplateFixHandler());
  }

  /* コンパイル対象のファイルごとに呼ばれる関数 */
  virtual void HandleTranslationUnit(clang::ASTContext &ast) {
    /* コード書き換えクラスの定義と設定 */
    clang::Rewriter rew;
    rew.setSourceMgr(cmpi->getSourceManager(), cmpi->getLangOpts());
    for (clang::Decl *D : ast.getTraversalScope()){
      D->dump();
    }
    /* ASTを探索するクラス */
    MyASTVisitor visitor(rew, ast);
    visitor.TraverseAST(ast); /* 探索開始 */
    visitor.AddAllocFuncAtLast();
    /* 対象ファイルのID取得*/
    clang::FileID id = cmpi->getSourceManager().getMainFileID();
    /* 書き換え結果の取得 (ASTVisitorが書き換えたこと前提) */
    const clang::RewriteBuffer *rbuf = rew.getRewriteBufferFor(id);
    std::cout <<"#include \"xmp_func_decl.h\""<<std::endl;
    std::cout <<"#include \"xmp_index_macro.h\""<<std::endl;
    std::cout <<"#include \"xmp_comm_macro.h\""<<std::endl;
    if (rbuf != nullptr) {
      std::string buf(rbuf->begin(), rbuf->end());
      std::cout << buf;
      std::cout << visitor.getEpiloge();
    } else {
      std::cerr << "Original code is unchanged" << std::endl;
    }
  }
};

/* コンパイラのインスタンス */
class MyFrontendAction : public clang::SyntaxOnlyAction {
public:
  /* ASTを受取るクラスの生成 */
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef file) {
    return std::make_unique<MyASTConsumer>(&ci);
  }
};

static llvm::cl::OptionCategory MinicOptionCategory("Mini-c Options");
static llvm::cl::extrahelp CommonHelp("hoge\n");
static llvm::cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser op(argc, argv, MinicOptionCategory);

  clang::tooling::ClangTool tool(op.getCompilations(), op.getSourcePathList());

#if 1
  /* コンパイラ起動時のオプション設定 */
  tool.clearArgumentsAdjusters();
  tool.appendArgumentsAdjuster(clang::tooling::getClangSyntaxOnlyAdjuster());
  tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(
      "-Wno-unused-value", clang::tooling::ArgumentInsertPosition::BEGIN));
  // avoid including g++ header files
#ifdef XEV_INCLUDE_PATH
  /* GNU Cがインストールされているとエラーが出るのでおまじない */
  tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(
      XEV_INCLUDE_PATH, clang::tooling::ArgumentInsertPosition::BEGIN));
#endif
#endif

  /* FrontendActionクラスの派生形に合わせてコンパイラ起動 */
  std::unique_ptr<clang::tooling::FrontendActionFactory> FrontendFactory;
  FrontendFactory =
      clang::tooling::newFrontendActionFactory<MyFrontendAction>();
  return tool.run(FrontendFactory.get());
}
