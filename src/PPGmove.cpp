#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;
 

void PragmaGmoveHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok){
    clang::SmallVector<clang::Token,1>  TokenList;
    PPUtil::TokenList TL;
    TripleList TPL;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    clang::SourceLocation EndLoc;
    std::string name;
    clang::Token Tok;
    clang::Token LHSTok;
    clang::Token RHSTok;
    PPUtil::TripleList LHSTPL,RHSTPL;
    clang::Token AsyncTok;
    std::string idstr;
    int movetype = 0;
    bool hasasync = false;

    name = "__xmp_gmove"+std::to_string(nodes);
    nodes++;
    PP.Lex(Tok);
    if(Tok.is(clang::tok::eod))
      goto parsenext;
    if(!Tok.is(clang::tok::identifier)){
      goto error;
    }
    idstr = Tok.getIdentifierInfo()->getName().str();
    if(idstr == "in")
      movetype = 1;
    else if(idstr == "out")
      movetype = 2;
    PP.Lex(Tok);
    if(Tok.is(clang::tok::eod))
      goto parsenext;
    idstr = Tok.getIdentifierInfo()->getName().str();
    if(idstr == "async"){
      if(!ParseAsync(PP, AsyncTok))
	goto error;
      hasasync = true;
    }
    PP.Lex(Tok);
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
 parsenext:
    /*Parse next*/
    if(!LHSTok.is(clang::tok::identifier)){
      goto error;
    }

    ArrayParser(PP, TL, LHSTPL, false);
    PP.Lex(Tok);
    if(!Tok.is(clang::tok::equal)){
      goto error;
    }
    if(!RHSTok.is(clang::tok::identifier)){
      goto error;
    }
    ArrayParser(PP, TL, RHSTPL, false);
    {
      AddVar(PP, TokenList, name, StartLoc);
      AddEndBrace(TokenList, EndLoc);
    }
    return;
 error:
    /*variables srcnoderef, destnoderef*/
    PP.DiscardUntilEndOfDirective();
};

