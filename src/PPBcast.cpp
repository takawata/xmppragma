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
    bool hasFrom = false ;
    bool hasTo = false;
    bool hasasync = false;
    clang::Token Tok;
    clang::Token AsyncTok;
    clang::SmallVector<clang::Token, 1> nodeToks;
    PPNodeRef From(PP);
    PPNodeRef To(PP);
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
	hasFrom = true;
	From.Parse(false);
      }
      if(idstr == "on"){
	hasTo = true;
	To.Parse(false);
      }
      if(idstr == "async"){
	if(!ParseAsync(PP, AsyncTok))
	  goto error;
	hasasync = true;
      }
    }
    EndLoc = Tok.getLocation();
    {
      llvm::errs()<<"E\n";
      for(auto &varnode : nodeToks){
	name = "__xmp_bcast"+std::to_string(nodes);
	nodes++;
	AddVar(PP, TokenList, name, StartLoc);
	AddTokenPtrElem(TokenList, varnode);
	AddEndBrace(TokenList,EndLoc);
      }
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
}
