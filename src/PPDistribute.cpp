#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;


void PragmaDistributeHandler::HandlePragma(clang::Preprocessor &PP,
					 clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok){
    int numtoken = 0;
    clang::Token distTok;
    clang::Token nameTok;
    clang::Token Tok;
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::SmallVector<clang::Token,1>  Params;
    clang::tok::TokenKind expected;
    clang::Token zeroToken;
    clang::SourceLocation StartLoc = FirstTok.getLocation();    
    clang::IdentifierInfo *II;
    std::string idstr;
    std::string name;
    clang::SourceLocation EndLoc;
    name = std::string("__xmp_distribute") + std::to_string(nodes);
    nodes++;
    SetNumericConstant(zeroToken, "0");      
    PP.Lex(distTok);


    if(!distTok.is(expected = clang::tok::identifier)){
      goto error;
    }
    PP.Lex(Tok);
    if(!Tok.is((expected = clang::tok::l_square))){
      goto error;
    }
    while(Tok.is(clang::tok::l_square)){
      PP.Lex(Tok);
      if(Tok.is(clang::tok::star)){
	Params.push_back(zeroToken);
	Params.push_back(zeroToken);
	PP.Lex(Tok);
	if(!Tok.is(expected = clang::tok::r_square)){
	  goto error;
	}
	continue;
      } else  if(!Tok.is(expected = clang::tok::identifier)){
	goto error;
      }
      idstr = Tok.getIdentifierInfo()->getName().str();
      if(idstr == "block"){
	SetNumericConstant(Tok, "1");
	Params.push_back(Tok);
      }else if(idstr == "cyclic"){
	SetNumericConstant(Tok, "2");
	Params.push_back(Tok);
      }else if(idstr == "gblock"){
	SetNumericConstant(Tok, "3");
	Params.push_back(Tok);
      }else{
	goto error;
      }
      PP.Lex(Tok);
      if(Tok.is(clang::tok::l_paren)){
	PP.Lex(Tok);
	//if(!gblock)
	if(!Tok.is(expected = clang::tok::numeric_constant)){
	  goto error;
	}
	Params.push_back(Tok);
	PP.Lex(Tok);
	if(!Tok.is(expected = clang::tok::r_paren)){
	  goto error;
	}
      }else{
	Params.push_back(zeroToken);
      }
      if(!Tok.is(expected = clang::tok::r_square)){
	goto error;
      }
      PP.Lex(Tok);
    }
    
    if(!Tok.is(expected = clang::tok::identifier)){
      goto error;
    }
    idstr = Tok.getIdentifierInfo()->getName().str();
    if(idstr != "onto"){
      goto error;
    }
    PP.Lex(nameTok);
    if(!nameTok.is(expected = clang::tok::identifier)){
      goto error;
    }
    /*Discard tokens to eod*/
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
    EndLoc = Tok.getLocation();
    /*Construct void * array*/
    {
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, distTok);
      AddTokenPtrElem(TokenList, nameTok);
      for(auto &&element : Params){
	AddVoidCastToken(TokenList, element);
	Tok.startToken();
	Tok.setKind(clang::tok::comma);
	TokenList.push_back(Tok);
      }
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

