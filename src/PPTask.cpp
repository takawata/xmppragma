#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;

void PragmaTaskHandler::HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) {
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    std::string name = std::string("__xmp_task")+std::to_string(nodes);
    clang::tok::TokenKind expected;
    clang::SourceLocation EndLoc;    
    clang::Token nodeTok;
    std::string idstr;
    clang::Token Tok;
    nodes++;
    PP.Lex(Tok);

    if(!Tok.is(expected = clang::tok::identifier)){
      goto error;
    }
    idstr = Tok.getIdentifierInfo()->getName().str();
    if(idstr != "on"){
      goto error;
    }
    PP.Lex(nodeTok);
    if(!nodeTok.is(expected = clang::tok::identifier)){
      goto error;
    }

    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
    EndLoc = Tok.getLocation();
    {
      AddVar(PP, TokenList, name, StartLoc);
      AddVoidCastToken(TokenList, nodeTok);
      AddEndBrace(TokenList, EndLoc);
      auto TokenArray = std::make_unique<clang::Token[]>(TokenList.size());
      std::copy(TokenList.begin(), TokenList.end(), TokenArray.get());
      PP.EnterTokenStream(std::move(TokenArray), TokenList.size(),
			/*DisableMacroExpansion=*/false,
			/*IsReinject=*/false);
    }
    return;
  error:
    PP.Diag(Tok.getLocation(), clang::diag::err_expected) << expected;
    PP.DiscardUntilEndOfDirective();
    return;
};
