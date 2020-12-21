#ifndef PPUTIL_H
#define PPUTIL_H
namespace PPUtil{
  void AddVoidPtr(clang::Preprocessor &PP,
		  clang::SmallVector<clang::Token, 1> &TokenList,
		  clang::Token &nameTok);
  void AddVar(clang::Preprocessor &PP,
	      clang::SmallVector<clang::Token, 1> &TokenList,
	      std::string name,
	      clang::SourceLocation &Loc
	      );
  
  void CreateUIntToken(clang::Preprocessor &PP,
		       clang::Token &Tok, unsigned int val,
		       clang::SourceLocation Start,
		       clang::SourceLocation End);
  void SetNumericConstant(clang::Token &Tok, const char *number);
  
  bool ArrayParser(clang::Preprocessor &PP,
		   std::vector<std::pair<clang::Token, clang::Token>> &arrays,
		   clang::Token &Tok);
  void AddVoidCastToken(clang::SmallVector<clang::Token, 1> &TokenList,
			clang::Token &myTok);
  void AddTokenPtrElem(clang::SmallVector<clang::Token, 1> &TokenList,
		       clang::Token &elemTok);
  void AddEndBrace(clang::SmallVector<clang::Token, 1> &TokenList);
}

#endif
