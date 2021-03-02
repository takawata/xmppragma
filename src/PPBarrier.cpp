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
	std::string idstr;
	PPNodeRef nodeRef(PP);
	clang::Token Tok;
	clang::tok::TokenKind expected;
	name = "__xmp_barrier"+std::to_string(nodes);
	nodes++;
	PP.Lex(Tok);

	if(!Tok.is(clang::tok::eod)){
	  /*Parse template reference*/
	  if(!Tok.is(expected = clang::tok::identifier)){
	    goto error;
	  }
	  idstr = Tok.getIdentifierInfo()->getName().str();
	  if(idstr != "on"){
	    goto error;
	  }
	  nodeRef.Parse(false);
	}

	/*Discard the rest*/
	PP.Lex(Tok);
	while(!Tok.is(clang::tok::eod)){
		PP.Lex(Tok);
	}
	{
		if(nodeRef.isValid()){
			nodeRef.outputDefinition(TokenList);
			nodeRef.outputReference(Tok);
		}else{
			SetNumericConstant(Tok, "0");
		}
		AddVar(PP, TokenList, name, StartLoc);
		if(nodeRef.isValid()){
			AddTokenPtrElem(TokenList, Tok);
		}else{
			AddVoidCastToken(TokenList, Tok);
		}
		AddEndBrace(TokenList, EndLoc);
	}
	return ;
 error:
	/*variables srcnoderef, destnoderef*/
	PP.DiscardUntilEndOfDirective();
}
