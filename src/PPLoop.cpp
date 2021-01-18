#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"
#include "PPclass.h"
using namespace PPUtil;
static bool ScanForLoop(clang::Preprocessor &PP,
			clang::SmallVector<clang::Token,1>  &TokenList,
			 clang::SmallVector<clang::Token,1> &LoopVarList,
			clang::tok::TokenKind &expected, clang::Token &Tok);

static bool SkipParen(clang::Preprocessor &PP,
		      clang::SmallVector<clang::Token,1>  &TokenList,
		      clang::tok::TokenKind &expected)
{
  clang::Token Tok;
  PP.Lex(Tok);
  if(!Tok.is(expected = clang::tok::l_paren))
    return false;
  TokenList.push_back(Tok);
  int level = 1;
  do{
    PP.Lex(Tok);
    TokenList.push_back(Tok);
    if(Tok.is(clang::tok::l_paren))
      level++;
    if(Tok.is(clang::tok::r_paren))
      level--;
  }while(level > 0);
  return true;
}
static bool ParseStmt(clang::Preprocessor &PP,
		      clang::SmallVector<clang::Token,1>  &TokenList,
			 clang::SmallVector<clang::Token,1> &LoopVarList,
		      clang::tok::TokenKind &expected, clang::Token &Tok)
{
  clang::SmallVector<clang::Token, 1> PendList;
  int level;
  if(Tok.is(clang::tok::kw_if)||
     Tok.is(clang::tok::kw_while)){
    TokenList.push_back(Tok);
    if(SkipParen(PP, TokenList, expected)== false){
      return false;
    }
    PP.Lex(Tok);
    return ParseStmt(PP, TokenList,LoopVarList,expected, Tok);
  }
  if(Tok.is(clang::tok::kw_for)){
    if(ScanForLoop(PP, TokenList,LoopVarList, expected, Tok)== false){
      return false;
    }
    return true;
  }
  if(Tok.is(clang::tok::l_brace)){
    TokenList.push_back(Tok);
    level = 1;
    do{
      PP.Lex(Tok);
      if(Tok.is(clang::tok::kw_for)){
	if(ScanForLoop(PP, TokenList,LoopVarList, expected, Tok)== false){
	  return false;
	}
	continue;
      }
      TokenList.push_back(Tok);
      if(Tok.is(clang::tok::l_brace)){
	level++;
	continue;
      }
      if(Tok.is(clang::tok::r_brace)){
	level--;
	continue;
      }
    }while(level > 0);
    return true;
  }

  TokenList.push_back(Tok);
  do{
    PP.Lex(Tok);
    if(Tok.is(clang::tok::l_paren)){
      ParseStmt(PP, TokenList, LoopVarList, expected, Tok);
    }else{
      TokenList.push_back(Tok);
    }
  }while(!Tok.is(clang::tok::semi));
  return true;
}
static bool ScanForLoop(clang::Preprocessor &PP,
			clang::SmallVector<clang::Token,1>  &TokenList,
			 clang::SmallVector<clang::Token,1> &LoopVarList,
			clang::tok::TokenKind &expected, clang::Token &Tok)
{
  clang::SmallVector<clang::Token, 1> PendList;
  int level;
  if(!Tok.is(expected = clang::tok::kw_for)){
    return false;
  }
  TokenList.push_back(Tok);
  PP.Lex(Tok);
  if(!Tok.is(expected = clang::tok::l_paren)){
    return false;
  }
  TokenList.push_back(Tok);
  do{
    PP.Lex(Tok);
    PendList.push_back(Tok);
  }while(!Tok.is(clang::tok::identifier)&&!Tok.is(clang::tok::semi));
  if(Tok.is(clang::tok::identifier)){
    auto result = std::find_if(LoopVarList.begin(), LoopVarList.end(),
			  [&Tok](clang::Token &VTok){
			    return
			      (Tok.getIdentifierInfo()->getName().str()
			       == VTok.getIdentifierInfo()->getName().str());});
    if(result == LoopVarList.end()){
      for(auto it = PendList.begin(); it != PendList.end(); it++){
	TokenList.push_back(*it);
      }
    }else{
      TokenList.push_back(Tok);
    }
  }
  do{
    PP.Lex(Tok);
    TokenList.push_back(Tok);
  }while(Tok.is(clang::tok::semi));
  level = 1;
  do{
    PP.Lex(Tok);
    if(Tok.is(clang::tok::l_paren))
      level++;
    if(Tok.is(clang::tok::r_paren))
      level--;
    TokenList.push_back(Tok);
  }while(level > 0);
  PP.Lex(Tok);
  return ParseStmt(PP, TokenList, LoopVarList, expected, Tok);
}

void PragmaLoopHandler::HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) {
    clang::Token Tok;
    clang::Token NodeTok;
    clang::Token ReductionTok;
    clang::tok::TokenKind expected;
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::SmallVector<clang::Token,1> LoopVarList;
    std::vector<std::pair<clang::Token,clang::Token>> arrays;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    clang::SourceLocation EndLoc;
    std::string name;
    int reductionKind = 0;
    int hasvarlist;
    name = std::string("__xmp_loop") + std::to_string(nodes);
    nodes++;
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
      PP.Lex(Tok);
    }

    /*Reduction */
    if(Tok.is(clang::tok::identifier)){
      if(Tok.getIdentifierInfo()->getName().str() != "reduction"){
	goto error;
      }
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
    }

    /*Discard tokens to eod*/
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
    EndLoc = Tok.getLocation();
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
      CreateUIntToken(PP, Tok, LoopVarList.size(), StartLoc, EndLoc);
      AddVoidCastToken(TokenList, Tok);
      Tok.startToken();
      Tok.setKind(clang::tok::comma);
      TokenList.push_back(Tok);
      for(auto &&LV : LoopVarList){
	AddTokenPtrElem(TokenList, LV);
      }
      if(reductionKind != 0){
	llvm::errs()<<"Reduction Found\n";
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
      }
      AddEndBrace(TokenList, EndLoc);
      PP.Lex(Tok);
      ScanForLoop(PP, TokenList, LoopVarList, expected, Tok);
      //Scan For loop
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
