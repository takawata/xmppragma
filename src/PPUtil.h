#ifndef PPUTIL_H
#define PPUTIL_H
#include <clang/Lex/Token.h>
namespace PPUtil{
  using TokenList = clang::SmallVector<clang::Token, 1>;
  struct Triple {
    int begin1;
    int end1;
    int begin2;
    int end2;
    int begin3;
    int end3;
  };
  using TripleList = clang::SmallVector<Triple, 1>;
  bool ArrayParser(clang::Preprocessor &PP, TokenList &TL, TripleList &TPL,
		   TokenList &ArgVarList, bool ignoreIdent);
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
  void AddEndBrace(clang::SmallVector<clang::Token, 1> &TokenList,
		   clang::SourceLocation End);
  int getReductionKind(clang::Token Tok);
  bool ParseAsync(clang::Preprocessor &PP, clang::Token &AsyncTok);
}
class PPNodeRef{
  PPUtil::TripleList TPL;
  PPUtil::TokenList TL;
  PPUtil::TokenList varList;
  clang::Preprocessor &PP;
  clang::Token nodeTok;
  std::string nodename;
  bool ready;
  static int nodes;
public:
  PPNodeRef(clang::Preprocessor &P):PP(P),ready(false)
  {
  };
  bool isValid()
  {return ready;};
  bool Parse(bool ignoreIdent);
  bool outputDefinition(llvm::SmallVector<clang::Token, 1> &TokenList);
  bool outputReference(clang::Token &Tok);
  PPUtil::TokenList::iterator varsBegin(){return varList.begin();};
  PPUtil::TokenList::iterator varsEnd(){return varList.end();};
};
#endif
