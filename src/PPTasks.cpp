#include "Rewriter.h"
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;

void PragmaTasksHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok) {
  clang::SmallVector<clang::Token,1>  TokenList;
  clang::SourceLocation StartLoc = FirstTok.getLocation();
  std::string name = std::string("__xmp_tasks")+std::to_string(nodes);
  
  nodes++;
  AddVar(PP, TokenList, name, StartLoc);
  AddEndBrace(TokenList);
  PP.DiscardUntilEndOfDirective();
  auto TokenArray = std::make_unique<clang::Token[]>(TokenList.size());
  std::copy(TokenList.begin(), TokenList.end(), TokenArray.get());
  PP.EnterTokenStream(std::move(TokenArray), TokenList.size(),
			/*DisableMacroExpansion=*/false,
		      /*IsReinject=*/false);
};

