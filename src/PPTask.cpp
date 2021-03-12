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
    PPNodeRef nodeRef(PP);
    nodes++;
    PP.Lex(Tok);

    if(!Tok.is(expected = clang::tok::identifier)){
      goto error;
    }
    idstr = Tok.getIdentifierInfo()->getName().str();
    if(idstr != "on"){
      goto error;
    }

    nodeRef.Parse(false);
    PP.Lex(Tok);

    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }


    EndLoc = Tok.getLocation();
    {
      nodeRef.outputDefinition(TokenList);
      nodeRef.outputReference(nodeTok);
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, nodeTok);
      /**/
      AddEndBrace(TokenList, EndLoc);
      /*Add "if(__xmp_task0)" */
      
      Tok.startToken();
      Tok.setKind(clang::tok::kw_if);
      TokenList.push_back(Tok);
      Tok.startToken();
      Tok.setKind(clang::tok::l_paren);
      TokenList.push_back(Tok);
      Tok.setKind(clang::tok::identifier);
      Tok.setIdentifierInfo(PP.getIdentifierInfo(name.c_str()));
      TokenList.push_back(Tok);
      Tok.startToken();
      Tok.setKind(clang::tok::r_paren);
      TokenList.push_back(Tok);

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
