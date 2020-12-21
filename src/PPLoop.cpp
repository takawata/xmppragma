#include "Rewriter.h"
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;


void PragmaLoopHandler::HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) {
    clang::Token Tok;
    clang::Token NodeTok;
    clang::tok::TokenKind expected;
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::SmallVector<clang::Token,1> LoopVarList;
    std::vector<std::pair<clang::Token,clang::Token>> arrays;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    clang::SourceLocation EndLoc;
    std::string name;

    int hasvarlist;
    name = std::string("__xmp_loop") + std::to_string(nodes);
    PP.Lex(Tok);

    if(Tok.is(clang::tok::l_paren)){
      /*Parse loop var list*/
      hasvarlist = 1;
      for(;;){
	PP.Lex(Tok);
	if(!Tok.is(expected = clang::tok::identifier)){
	  goto error;
	}
	LoopVarList.push_back(Tok);
	PP.Lex(Tok);
	if(Tok.is(clang::tok::r_paren)){
	  break;
	}else if(!Tok.is(expected = clang::tok::comma)){
	  goto error;
	}
      }
    }
    PP.Lex(Tok);
    /* on */
    if(!Tok.is(expected = clang::tok::identifier)){
      goto error;
    }

    if(Tok.getIdentifierInfo()->getName().str() != "on"){
      goto error;
    }
    /* node or template ref*/
    PP.Lex(NodeTok);
    if(!NodeTok.is(expected = clang::tok::identifier)){
      goto error;
    }
    PP.Lex(Tok);
    while(Tok.is(clang::tok::l_square)){
      PP.Lex(Tok);
      if(!Tok.is(expected = clang::tok::identifier)){
	goto error;
      }
      if(!hasvarlist){
	LoopVarList.push_back(Tok);
      }

      PP.Lex(Tok);
      if(!Tok.is(expected = clang::tok::r_square)){
	goto error;
      }

    }
    /*Discard tokens to eod*/
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }

    /*Construct void * array*/
    {
      /*{*/
      Tok.startToken();
      Tok.setKind(clang::tok::l_brace);
      TokenList.push_back(Tok);
      Tok.startToken();
      /*define loop variable decl.*/
      for(auto &&LV : LoopVarList){
	Tok.startToken();
	Tok.setKind(clang::tok::kw_int);
	TokenList.push_back(Tok);
	TokenList.push_back(LV);
	Tok.startToken();
	Tok.setKind(clang::tok::semi);
	TokenList.push_back(Tok);
      }

      /*Add descriptor array decl.*/
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, NodeTok);
      for(auto &&LV : LoopVarList){
	AddTokenPtrElem(TokenList, LV);
      }
      AddEndBrace(TokenList);
      //Scan For loop
      expected = clang::tok::kw_for;
      enum scanstate{wait_lparen,wait_for_rparen, wait_rparen,
		     ignore_type, wait_ident, wait_single, none};
      scanstate state = none;
      int ignore = 0;
      int level = 0;
      int paren_level = 0;
      do{
	PP.Lex(Tok);
	if((expected != clang::tok::unknown)&&
	   !Tok.is(expected))
	  goto error;
	switch(Tok.getKind()){
	case clang::tok::kw_if:
	case clang::tok::kw_while:
	  if(state != wait_single){
	    level++;
	  }
	  state = none;
	  expected = clang::tok::l_paren;
	  break;
	case clang::tok::kw_for:
	  if(state != wait_single){
	    level++;
	  }
	  expected = clang::tok::l_paren;
	  state = wait_lparen;
	  break;
	case clang::tok::l_paren:
	  paren_level++;
	  if(state == wait_lparen){
	    state = ignore_type;
	  }else {
	    state = wait_rparen;
	  }
	  expected = clang::tok::unknown;
	  break;
	case clang::tok::r_paren:
	  paren_level--;
	  if(paren_level < 0){
	    goto error;
	  }
	  if(paren_level == 0){
	    if(state == wait_rparen){
	      state = wait_single;
	    }
	  }
	  expected = clang::tok::unknown;
	  break;
	case clang::tok::l_brace:
	  if(state == wait_single){
	    state = none;
	  }else{
	    level++;
	  }
	  expected = clang::tok::unknown;
	  break;
	case clang::tok::r_brace:
	  level--;
	  expected = clang::tok::unknown;
	case clang::tok::identifier:
	  if(state == wait_ident){
	    ignore = 0;
	    state = wait_rparen;
	  }
	  expected = clang::tok::unknown;
	  break;
	case clang::tok::semi:
	  if(state == wait_single){
	    state = none;
	    level --;
	  }
	  expected = clang::tok::unknown;
	  break;
	default:
	  if(state == ignore_type){
	    ignore = 1;
	    state = wait_ident;
	  }
	  expected = clang::tok::unknown;
	  break;
	}
	if(!ignore)
	  TokenList.push_back(Tok);
      }while(level > 0);
      /* End local var block */
      Tok.startToken();
      Tok.setKind(clang::tok::r_brace);
      TokenList.push_back(Tok);
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
