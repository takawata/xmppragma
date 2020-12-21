#include "Rewriter.h"
#include "PPUtil.h"
#include "PPclass.h"

using namespace PPUtil;

void PragmaNodesHandler::HandlePragma(clang::Preprocessor &PP,
				      clang::PragmaIntroducer Introducer,
				      clang::Token &FirstTok) {
    int numtoken = 0;
    clang::Token Tok;
    clang::Token nameTok;
    clang::SmallVector<clang::Token,1> TokenList;
    std::vector<std::pair<clang::Token,clang::Token>> arrays;
    std::string name;
    PP.Lex(nameTok);
    clang::SourceLocation StartLoc = FirstTok.getLocation();

    if(!nameTok.isAnyIdentifier()){
      goto error;
    }
    AddVoidPtr(PP, TokenList, nameTok);
    
    name = std::string("__xmp_node") + std::to_string(nodes);
    nodes++;
    if(!ArrayParser(PP, arrays, Tok)){
      goto error;
    }
    //Discard to EOD manually.
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
    llvm::errs()<<"AA";        
    {    
      clang::SourceLocation EndLoc = Tok.getLocation();    
      llvm::errs()<<name<<"\n";
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, nameTok);
      
      for(auto &&element: arrays){
	AddVoidCastToken(TokenList, element.first);
	if(element.first.is(clang::tok::minus))
	  TokenList.push_back(element.second);
	Tok.startToken();
	Tok.setKind(clang::tok::comma);
	TokenList.push_back(Tok);
      }

      AddEndBrace(TokenList);
      
      auto TokenArray = std::make_unique<clang::Token[]>(TokenList.size());
      std::copy(TokenList.begin(), TokenList.end(), TokenArray.get());
      PP.EnterTokenStream(std::move(TokenArray), TokenList.size(),
			  /*DisableMacroExpansion=*/false, /*IsReinject=*/false);

    }
    
    return;
  error:
    PP.Diag(Tok.getLocation(), clang::diag::err_expected)
      << clang::tok::identifier;
    PP.DiscardUntilEndOfDirective();
    return;
}
