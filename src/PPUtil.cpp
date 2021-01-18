#include <clang/Lex/Preprocessor.h>
#include "PPUtil.h"

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
  llvm::errs()<<"SUCCESS"<<"\n";
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
