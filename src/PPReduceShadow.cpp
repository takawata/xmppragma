#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;


void PragmaReduceShadowHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok) {
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::Token Tok;
    PP.Lex(Tok);
    if(!Tok.is(clang::tok::identifier)){
      goto error;
    }
 error:
    PP.DiscardUntilEndOfDirective();
    return ;
};

