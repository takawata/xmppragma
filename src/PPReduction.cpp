#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;


void PragmaReductionHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok) {
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::Token Tok;
    TripleList TPL;
    clang::Token TargetTok;
    clang::Token ReductionTok;
    clang::Token nodeTok;
    clang::SourceLocation StartLoc;
    clang::SourceLocation EndLoc;
    bool hason = false;
    PPNodeRef nodeRef(PP);
    clang::SmallVector<clang::Token, 1> TL;
    std::string name;
    clang::tok::TokenKind expected;
    int reductionKind;

    name = "__xmp_reduction" + std::to_string(nodes);
    PP.Lex(Tok);
    if(!Tok.is(expected = clang::tok::l_paren))
      goto error;
    PP.Lex(Tok);
    if((reductionKind = getReductionKind(Tok)) == 0){
      goto error;
    }
    PP.Lex(Tok);
    if(!Tok.is(expected = clang::tok::colon)){
      goto error;
    }
    PP.Lex(ReductionTok);
    if(!ReductionTok.is(expected = clang::tok::identifier)){
      goto error;
    }
    PP.Lex(Tok);
    if(!Tok.is(expected = clang::tok::r_paren)){
      goto error;
    }
    PP.Lex(Tok);
    if(Tok.is(clang::tok::eod))
      goto end;
    if(!Tok.is(clang::tok::identifier)
       ||Tok.getIdentifierInfo()->getName().str() != "on"){
      goto error;
    }
    hason = true;
    nodeRef.Parse(false);

 end:
    {
      if(hason){
	nodeRef.outputDefinition(TokenList);
	nodeRef.outputReference(TargetTok);
      }
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, nodeTok);
      if(reductionKind<0){
	Tok.startToken();
	Tok.setKind(clang::tok::minus);
	AddVoidCastToken(TokenList,Tok);
	StartLoc = EndLoc = ReductionTok.getLocation();
	CreateUIntToken(PP, Tok, -reductionKind, StartLoc, EndLoc);
	TokenList.push_back(Tok);
      }else{
	  StartLoc = EndLoc = ReductionTok.getLocation();
	  CreateUIntToken(PP, Tok, reductionKind, StartLoc, EndLoc);
	  AddVoidCastToken(TokenList, Tok);
      }
      Tok.startToken();
      Tok.setKind(clang::tok::comma);
      TokenList.push_back(Tok);
      AddTokenPtrElem(TokenList, ReductionTok);
      if(hason){
	AddTokenPtrElem(TokenList, TargetTok);
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
    /*variables srcnoderef, destnoderef*/
    PP.DiscardUntilEndOfDirective();
};

