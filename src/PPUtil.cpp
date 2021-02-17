#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"

bool PPUtil::ParseAsync(clang::Preprocessor &PP,
		       clang::Token &AsyncTok)
{
	clang::Token Tok;
	PP.Lex(Tok);
	if(!Tok.is(clang::tok::l_paren))
		return false;
	PP.Lex(AsyncTok);
	if(!AsyncTok.is(clang::tok::numeric_constant)){
		return false;
	}
	PP.Lex(Tok);
	if(!Tok.is(clang::tok::r_paren))
		return false;
	return true;
}

void PPUtil::AddVoidPtr(clang::Preprocessor &PP,
		       clang::SmallVector<clang::Token, 1> &TokenList,
		       clang::Token &nameTok)
{
  clang::Token Tok;
  Tok.startToken();
  Tok.setKind(clang::tok::kw_void);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::star);
  TokenList.push_back(Tok);
  TokenList.push_back(nameTok);
  Tok.startToken();
  Tok.setKind(clang::tok::semi);
  TokenList.push_back(Tok);  
}
void PPUtil::AddVar(clang::Preprocessor &PP,
		   clang::SmallVector<clang::Token, 1> &TokenList,
		   std::string name,
		   clang::SourceLocation &Loc
		   )
{
  clang::Token Tok;
  Tok.startToken();
  Tok.setKind(clang::tok::kw_void);
  Tok.setLocation(Loc);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::star);
  TokenList.push_back(Tok);
  
  Tok.startToken();
  Tok.setKind(clang::tok::identifier);
  Tok.setIdentifierInfo(PP.getIdentifierInfo(name.c_str()));
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::l_square);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::r_square);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::equal);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::l_brace);
  TokenList.push_back(Tok);
  
}
void PPUtil::CreateUIntToken(clang::Preprocessor &PP,
			    clang::Token &Tok, unsigned int val,
			    clang::SourceLocation Start,
			    clang::SourceLocation End)
{
  std::string vstr = std::to_string(val);
  Tok.startToken();
  Tok.setKind(clang::tok::numeric_constant);
  PP.CreateString(vstr.c_str(), Tok, Start, End);
}
void PPUtil::SetNumericConstant(clang::Token &Tok, const char *number)
{
  Tok.startToken();
  Tok.setKind(clang::tok::numeric_constant);
  Tok.setLiteralData(number);
  Tok.setLength(std::strlen(number));
}
bool PPUtil::ArrayParser(clang::Preprocessor &PP,
			 TokenList &TL,
			 TripleList &TPL,
			 bool ignoreIdent)
{
  int v[6];
  clang::Token Tok;
  PP.Lex(Tok);

  if(!Tok.is(clang::tok::l_square))
    return false;
  for(;;){
    bool isbegin = true;
    if(!Tok.is(clang::tok::l_square)){
      PP.Backtrack();
      break;
    }
    PP.Lex(Tok);
    /* [*] */
    if(Tok.is(clang::tok::star)){
      Tok.startToken();
      Tok.setKind(clang::tok::minus);
      TL.push_back(Tok);
      v[0] =  TL.size() - 1;
      SetNumericConstant(Tok,"2");
      TL.push_back(Tok);
      v[1] = TL.size();
      TPL.push_back({v[0], v[1], v[0], v[1], v[0], v[1]});
      PP.Lex(Tok);
      if(!Tok.is(clang::tok::r_square))
	return false;
      PP.Lex(Tok);
      continue;
    }
    /* [:] */
    if(Tok.is(clang::tok::colon)){
      Tok.startToken();
      Tok.setKind(clang::tok::minus);
      TL.push_back(Tok);
      v[0] = TL.size() - 1;
      SetNumericConstant(Tok, "3");
      TL.push_back(Tok);
      v[1] = TL.size();
      TPL.push_back({v[0], v[1], v[0], v[1], v[0], v[1]});
      PP.Lex(Tok);
      if(!Tok.is(clang::tok::r_square))
	return false;
      PP.Lex(Tok);
      continue;
    }
    /*[expr(:expr:expr)]*/
    int i = 0;
    for(;;){
      if(Tok.is(clang::tok::identifier)&& ignoreIdent){
	/*erasure ident to avoid compile error*/
	llvm::errs()<<"IgnoreIdent\n";
	SetNumericConstant(Tok, "0");
      }
      if(Tok.is(clang::tok::r_square)){
	v[i] = TL.size();
	i++;
	llvm::errs()<<v[i]<<","<<i<<"\n";
	if(i == 2){
	  v[2] = v[4] = TL.size();
	  Tok.startToken();
	  Tok.setKind(clang::tok::minus);
	  TL.push_back(Tok);
	  SetNumericConstant(Tok, "1");
	  TL.push_back(Tok);
	  v[3] = v[5] = TL.size();
	}else if(i == 4){
	  v[4] = TL.size();
	  SetNumericConstant(Tok, "1");
	  TL.push_back(Tok);
	  v[5] = TL.size();
	}
	TPL.push_back({v[0], v[1], v[2], v[3], v[4], v[5]});
	llvm::errs()<<v[1]- v[0]<<","<<v[3] -v[2]<<","<<v[5] - v[4]<<"\n";
	PP.EnableBacktrackAtThisPos();
	PP.Lex(Tok);
	break;
      }
      if(Tok.is(clang::tok::colon)){
	v[i] = TL.size();
	isbegin= true;
	i++;
      }
      TL.push_back(Tok);
      if(isbegin){
	v[i] = TL.size()-1;
	isbegin = false;
	i++;
      }
      PP.Lex(Tok);
    }
  }
  return true;
}
bool PPUtil::ArrayParser(clang::Preprocessor &PP,
			std::vector<std::pair<clang::Token, clang::Token>> &arrays,
			clang::Token &Tok)
{
  PP.Lex(Tok);
  if(!Tok.is(clang::tok::l_square))
    return false;
  while(Tok.is(clang::tok::l_square)){
    clang::Token x,y;
    bool needrange = false;
    PP.Lex(Tok);
    if(Tok.is(clang::tok::numeric_constant)){
      x = Tok;
      needrange = true;
    }else if(Tok.is(clang::tok::star)){
      x.startToken();
      x.setKind(clang::tok::minus);
      SetNumericConstant(y, "1");
    }else if(Tok.is(clang::tok::colon)){
      x.startToken();
      x.setKind(clang::tok::minus);
      SetNumericConstant(y, "2");
    }else{
      return false;
    }

    PP.Lex(Tok);
    if(needrange && Tok.is(clang::tok::colon)){
      PP.Lex(Tok);
      if(!Tok.is(clang::tok::numeric_constant)){
	return false;
      }
      y = Tok;
      PP.Lex(Tok);
    }
    if(!Tok.is(clang::tok::r_square)){
      return false;
    }
    arrays.push_back(std::make_pair(x,y));
    PP.Lex(Tok);    
  }
  return true;
}
		       
void PPUtil::AddVoidCastToken(clang::SmallVector<clang::Token, 1> &TokenList,
			     clang::Token &myTok)
{
  clang::Token Tok;
  Tok.startToken();
  Tok.setKind(clang::tok::l_paren);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::kw_void);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::star);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::r_paren);
  TokenList.push_back(Tok);
  TokenList.push_back(myTok);
 
}
int PPUtil::getReductionKind(clang::Token Tok)
{
      switch(Tok.getKind()){
      case clang::tok::plus:
      case clang::tok::star:
      case clang::tok::amp:
      case clang::tok::pipe:
      case clang::tok::ampamp:
      case clang::tok::pipepipe:
	return Tok.getKind();
      case clang::tok::identifier:
	{
	  auto str = Tok.getIdentifierInfo()->getName().str();
	  std::vector<const char*> keywords = {"max", "min", "firstmax",
					 "firstmin", "lastmax", "lastmin"};
	  auto result = std::find(keywords.begin(), keywords.end(), str);
	  if(result != keywords.end()){
	    return -((result - keywords.begin())+1);
	  }else{
	    return 0;
	  }
	}
      default:
	return 0;
      }
      return 0;
}
void PPUtil::AddTokenPtrElem(clang::SmallVector<clang::Token, 1> &TokenList,
			 clang::Token &elemTok)
{
  clang::Token Tok;
  Tok.startToken();
  Tok.setKind(clang::tok::amp);
  TokenList.push_back(Tok);
  TokenList.push_back(elemTok);
  Tok.startToken();
  Tok.setKind(clang::tok::comma);
  TokenList.push_back(Tok);
}
void PPUtil::AddEndBrace(clang::SmallVector<clang::Token, 1> &TokenList,
			 clang::SourceLocation EndLoc)
{
  clang::Token Tok;
  Tok.startToken();
  Tok.setKind(clang::tok::r_brace);
  Tok.setLocation(EndLoc);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::semi);
  TokenList.push_back(Tok);
}

int PPNodeRef::nodes = 0;

bool PPNodeRef::Parse(bool ignoreIdent)
{
  clang::Token Tok;

  nodename = "__xmp_nref"+std::to_string(nodes);
  nodes++;
  PP.Lex(nodeTok);
  if(!nodeTok.is(clang::tok::identifier)){
    ready = false;
    return false;
  }

  if(!PPUtil::ArrayParser(PP, TL, TPL, ignoreIdent)){
    return false;
  }
  ready = true;

  return true;
}

bool PPNodeRef::outputDefinition(clang::SmallVector<clang::Token, 1> &TokenList)
{
  clang::Token Tok;
  clang::SourceLocation Loc;
  if(!ready)
    return false;

  PPUtil::AddVar(PP, TokenList, nodename, Loc);
  PPUtil::AddTokenPtrElem(TokenList, nodeTok);
  llvm::errs()<<"TPLSIZE"<<TPL.size()<<"\n";
  PPUtil::CreateUIntToken(PP, Tok, TPL.size(), Loc, Loc);
  PPUtil::AddVoidCastToken(TokenList, Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::comma);
  TokenList.push_back(Tok);
  for(auto &tple : TPL){
    PPUtil::AddVoidCastToken(TokenList, TL[tple.begin1]);
    for(int i = tple.begin1 + 1;  i < tple.end1; i++){
	  TokenList.push_back(TL[i]);
    }
    Tok.startToken();
    Tok.setKind(clang::tok::comma);
    TokenList.push_back(Tok);
    PPUtil::AddVoidCastToken(TokenList, TL[tple.begin2]);
    for(int i = tple.begin2 + 1;  i < tple.end2; i++){
      llvm::errs()<<TL[i].getName();
      TokenList.push_back(TL[i]);
    }
    Tok.startToken();
    Tok.setKind(clang::tok::comma);
    TokenList.push_back(Tok);
    PPUtil::AddVoidCastToken(TokenList, TL[tple.begin3]);
    for(int i = tple.begin3 + 1;  i < tple.end3; i++){
      TokenList.push_back(TL[i]);
    }
    Tok.startToken();
    Tok.setKind(clang::tok::comma);
    TokenList.push_back(Tok);
  }
  PPUtil::AddEndBrace(TokenList, Loc);

  return true;
}
bool PPNodeRef::outputReference(clang::Token &Tok)
{
  if(!ready)
    return false;
  Tok.setKind(clang::tok::identifier);
  Tok.setIdentifierInfo(PP.getIdentifierInfo(nodename.c_str()));
  return true;
}
