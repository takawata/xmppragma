#include "Rewriter.h"
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;


void PragmaArrayHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok) {
    clang::SmallVector<clang::Token,1>  TokenList;

    PP.DiscardUntilEndOfDirective();
};

