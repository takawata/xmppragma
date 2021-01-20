#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;


void PragmaReflectHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok) {
    clang::SmallVector<clang::Token,1>  TokenList;
    std::string name;
    clang::Token Tok;
#if 0
    /*reflect should support multiple variables.*/
    clang::Token nodeToks;
#else
    clang::Token nodeTok;
#endif
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    clang::SourceLocation EndLoc;
    clang::tok::TokenKind expected;

    name = "__xmp_reflect" + std::to_string(nodes);
    nodes++;
    PP.Lex(Tok);
    if(!Tok.is(expected = clang::tok::l_paren))
      goto error;
    PP.Lex(nodeTok);
    if(!nodeTok.is(expected = clang::tok::identifier))
      goto error;
    PP.Lex(Tok);
    if(!Tok.is(expected = clang::tok::r_paren))
       goto error;

    /*Discard tokens to eod*/
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
    EndLoc = Tok.getLocation();

    {
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, nodeTok);
      AddEndBrace(TokenList, EndLoc);
      auto TokenArray = std::make_unique<clang::Token[]>(TokenList.size());
      std::copy(TokenList.begin(), TokenList.end(), TokenArray.get());
      PP.EnterTokenStream(std::move(TokenArray), TokenList.size(),
			  /*DisableMacroExpansion=*/false,
			  /*IsReinject=*/false);

      /*variables srcnoderef, destnoderef*/
    }
    return;
  error:
    PP.Diag(Tok.getLocation(), clang::diag::err_expected) << expected;
    PP.DiscardUntilEndOfDirective();
    return;
};

