#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;


void PragmaBcastHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok) {
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::SmallVector<std::pair<clang::Token, clang::Token>,1> arrays;
    std::string name;
    clang::SmallVector<clang::Token,1>  ArrayTokenList;
    TripleList FromTriple;
    TripleList ToTriple;
    bool hasFrom = false ;
    bool hasTo = false;
    bool hasasync = false;
    clang::Token Tok;
    clang::Token AsyncTok;
    clang::SmallVector<clang::Token, 1> nodeToks;
    clang::Token From;
    clang::Token To;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    clang::SourceLocation EndLoc;
    clang::tok::TokenKind expected;
    PP.Lex(Tok);
    if(!Tok.is(clang::tok::l_paren))
      goto error;
    PP.Lex(Tok);
    do{
      if(Tok.is(clang::tok::identifier)){
	nodeToks.push_back(Tok);
      }
      PP.Lex(Tok);
    }while(Tok.is(clang::tok::comma));
    if(!Tok.is(clang::tok::r_paren)){
      goto error;
    }
    PP.Lex(Tok);
    while(!Tok.is(clang::tok::eod)){
      if(!Tok.is(clang::tok::identifier))
	goto error;
      auto idstr = Tok.getIdentifierInfo()->getName().str();
      if(idstr == "from"){
	PP.Lex(From);
	if(!From.is(clang::tok::identifier)){
	  goto error;
	}
	hasFrom= true;
	ArrayParser(PP, ArrayTokenList, FromTriple, false);
      }
      if(idstr == "on"){
	hasTo = true;
	PP.Lex(From);
	if(!To.is(clang::tok::identifier)){
	  goto error;
	}
	hasTo= true;
	ArrayParser(PP, ArrayTokenList, ToTriple, false);
      }
      if(idstr == "async"){
	if(!ParseAsync(PP, AsyncTok))
	  goto error;
	hasasync = true;
      }
    }
    {
      for(auto &varnode : nodeToks){
	name = "__xmp_bcast"+std::to_string(nodes);
	nodes++;
	AddVar(PP, TokenList, name, StartLoc);
	AddTokenPtrElem(TokenList, varnode);
	AddEndBrace(TokenList,EndLoc);
      }
    }
    return;
 error:
    /*variables srcnoderef, destnoderef*/
    PP.DiscardUntilEndOfDirective();
}
