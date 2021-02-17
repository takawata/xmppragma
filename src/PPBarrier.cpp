#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;


void PragmaBarrierHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok) {
	clang::SmallVector<clang::Token,1>  TokenList;
	clang::SourceLocation StartLoc = FirstTok.getLocation();
	clang::SourceLocation EndLoc;
	std::string name;
	clang::Token Tok;
	name = "__xmp_barrier"+std::to_string(nodes);
	nodes++;
	/*Parse template reference*/
	/*Discard the rest*/
	PP.Lex(Tok);
	while(!Tok.is(clang::tok::eod)){
		PP.Lex(Tok);
	}
	{
		AddVar(PP, TokenList, name, StartLoc);
		AddEndBrace(TokenList, EndLoc);
	}
	return ;
 error:
	/*variables srcnoderef, destnoderef*/
	PP.DiscardUntilEndOfDirective();
}
