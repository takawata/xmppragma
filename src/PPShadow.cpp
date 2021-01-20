#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;


void PragmaShadowHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok) {
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::SmallVector<std::pair<clang::Token, clang::Token>,1> arrays;
    std::string name;
    clang::Token Tok;
    clang::Token nodeTok;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    clang::SourceLocation EndLoc;
    clang::tok::TokenKind expected;
    int dim = 0;
    name = "__xmp_shadow" + std::to_string(nodes);
    nodes++;
    PP.Lex(nodeTok);

    if(!nodeTok.is(expected=clang::tok::identifier))
      goto error;
    PP.Lex(Tok);

    while(Tok.is(clang::tok::l_square)){
      clang::Token Tok2;
      dim++;
      /*Expression*/
      PP.Lex(Tok2);
      if(Tok2.is(clang::tok::star)){
	SetNumericConstant(Tok, "0");
	arrays.push_back(std::make_pair(Tok, Tok));
	PP.Lex(Tok);
	if(!Tok.is(clang::tok::r_square)){
	  goto error;
	}
	PP.Lex(Tok);
	continue;
      }
      /*Colon?*/
      PP.Lex(Tok);
      if(!Tok.is(clang::tok::colon)){
	PP.Lex(Tok);
	if(!Tok.is(clang::tok::r_square)){
	  goto error;
	}
	arrays.push_back(std::make_pair(Tok2, Tok2));
	PP.Lex(Tok);
	continue;
      }
      PP.Lex(Tok);
      arrays.push_back(std::make_pair(Tok2,Tok));
      PP.Lex(Tok);
      if(!Tok.is(clang::tok::r_square)){
	goto error;
      }
      PP.Lex(Tok);
    }

    /*Discard tokens to eod*/
    while(!Tok.is(clang::tok::eod)){
      llvm::errs()<<Tok.getName()<<"\n";
      PP.Lex(Tok);
    }
    EndLoc = Tok.getLocation();
    {
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, nodeTok);
      CreateUIntToken(PP, Tok, dim, StartLoc, EndLoc);
      AddVoidCastToken(TokenList, Tok);
      for(auto &it : arrays){
	Tok.startToken();
	Tok.setKind(clang::tok::comma);
	TokenList.push_back(Tok);
	AddVoidCastToken(TokenList, it.first);
	Tok.startToken();
	Tok.setKind(clang::tok::comma);
	TokenList.push_back(Tok);
	AddVoidCastToken(TokenList, it.second);
      }
      AddEndBrace(TokenList, EndLoc);
      auto TokenArray = std::make_unique<clang::Token[]>(TokenList.size());
      std::copy(TokenList.begin(), TokenList.end(), TokenArray.get());
      PP.EnterTokenStream(std::move(TokenArray), TokenList.size(),
			  /*DisableMacroExpansion=*/false,
			  /*IsReinject=*/false);
    }

    return;
    /*variables ranges*/
  error:
    PP.Diag(Tok.getLocation(), clang::diag::err_expected) << expected;
    PP.DiscardUntilEndOfDirective();
};

