#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;
  
void PragmaAlignHandler::HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) {
    clang::Token alignTok;
    clang::Token tempTok;
    clang::Token Tok;
    clang::tok::TokenKind expected;
    clang::SmallVector<clang::Token,1>  TokenList;
    std::vector<int> arraykind;
    std::string idstr;
    std::string name;
    int arraydim = 0;
    int tempdim = 0;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    clang::SourceLocation EndLoc;
    name = std::string("__xmp_align") + std::to_string(nodes);    
    PP.Lex(alignTok);
    if(!alignTok.is(expected = clang::tok::identifier)){
      goto error;
    }
    PP.Lex(Tok);
    while(Tok.is(clang::tok::l_square)){
      arraydim ++;
      PP.Lex(Tok);
      if(Tok.is(clang::tok::star)){
	arraykind.push_back(1);
      }else if(Tok.is(clang::tok::colon)){
	arraykind.push_back(2);
      } else if(Tok.is(expected = clang::tok::identifier)){
	arraykind.push_back(0);
      }else{
	goto error;
      }
      PP.Lex(Tok);
      if(!Tok.is(expected = clang::tok::r_square)){
	goto error;
      }
      PP.Lex(Tok);
    }
    if(!Tok.is(expected = clang::tok::identifier)){
      goto error;
    }
    idstr = Tok.getIdentifierInfo()->getName().str();
    if(idstr != "with"){
      goto error;
    }
    PP.Lex(tempTok);
    if(!tempTok.is(expected = clang::tok::identifier)){
      goto error;
    }

    PP.Lex(Tok);
    while(Tok.is(clang::tok::l_square)){
      int offs = 0;
      PP.Lex(Tok);
      if(tempdim >=arraydim){
	if(!Tok.is(expected = clang::tok::star)){
	  goto error;
	}
	PP.Lex(Tok);
	if(!Tok.is(expected = clang::tok::r_square)){
	  goto error;
	}
      }else if(Tok.is(clang::tok::identifier) && arraykind[tempdim] == 0){
	PP.Lex(Tok);
	while(!Tok.is(clang::tok::r_square)){
	  //TODO: evaluate token to r_square
	  PP.Lex(Tok);
	}
	arraykind[tempdim] =  offs;
      }else if(Tok.is(clang::tok::colon)&& arraykind[tempdim] == 2){
	PP.Lex(Tok);
	if(!Tok.is(expected = clang::tok::r_square)){
	  goto error;
	}
	arraykind[tempdim] =  offs;
      }
      tempdim ++;
      PP.Lex(Tok);
    }

    /* check for excess element*/
    for(int i = tempdim; i < arraydim; i++){
      if(arraykind[i] != 1){
	expected = clang::tok::star;
	goto error;
      }
    }
    llvm::errs()<<arraydim<<","<<tempdim<<"\n";
    /*Discard tokens to eod*/
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
    EndLoc = Tok.getLocation();
    /*Construct void * array*/
    {
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, alignTok);
      AddTokenPtrElem(TokenList, tempTok);
      CreateUIntToken(PP, Tok, arraydim, StartLoc, EndLoc);
      AddVoidCastToken(TokenList, Tok);
      Tok.startToken();
      Tok.setKind(clang::tok::comma);
      TokenList.push_back(Tok);
      CreateUIntToken(PP, Tok, tempdim, StartLoc, EndLoc);
      AddVoidCastToken(TokenList, Tok);
      Tok.startToken();
      Tok.setKind(clang::tok::comma);
      TokenList.push_back(Tok);

      for(int i = 0; i < std::min(arraydim, tempdim) ; i++){
	CreateUIntToken(PP, Tok, arraykind[i], StartLoc, EndLoc);
	AddVoidCastToken(TokenList, Tok);
	Tok.startToken();
	Tok.setKind(clang::tok::comma);
	TokenList.push_back(Tok);
      }
      AddEndBrace(TokenList, EndLoc);
      /* other params are not concerned for now*/
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
}
