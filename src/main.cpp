#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Version.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Lex/LexDiagnostic.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"
#include <utility>
#include <iostream>

/* AST traversal */
class MyASTVisitor : public clang::RecursiveASTVisitor<MyASTVisitor> {
  clang::Rewriter &rew;
  struct node{
    
  };
  clang::ASTContext &ast;
  struct align_vars{
    std::vector<clang::VarDecl*> vars;
  };
public:

  MyASTVisitor(clang::Rewriter &r,clang::ASTContext &a) : rew(r),ast(a) {}
  /* まず、pragmaを置き換えた変数を見つける */
  bool VisitVarDecl(clang::VarDecl *vdecl){
    std::string name = vdecl->getName().str();
    clang::Expr::EvalResult ev;
    clang::SourceLocation SL;
    if(name.find("__xmp_node") == 0){
      llvm::errs()<<"NODE"<<"\n";
      auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
      assert((content));
      auto node_stmt = llvm::dyn_cast<clang::UnaryOperator>
	(content->getInit(0)->IgnoreCasts());
      auto declref = llvm::dyn_cast<clang::DeclRefExpr>
	(node_stmt->getSubExpr());
      auto node_decl = declref->getDecl();
      node_decl->dump();
      auto VD = llvm::dyn_cast<clang::VarDecl>(node_decl);
      assert(VD);
      if(VD&&VD->isFunctionOrMethodVarDecl()){
	llvm::errs()<<"Local Decl\n";
      }
      VD->print(llvm::errs());
      int dim = content->getNumInits() - 1;
      llvm::errs()<<"Dimension"<< dim<<"\n";
      for(int i = 1; i < content->getNumInits(); i++){
	if( content->getInit(i)->IgnoreCasts()->EvaluateAsInt(ev, ast)){
	  llvm::errs()<<ev.Val.getInt()<<"\n";
	}else{
	  llvm::errs()<<"ConvErr"<<"\n";
	}
      }
      {
	SL = vdecl->getBeginLoc();
	auto &SM = rew.getSourceMgr();
	std::string codestr;
	llvm::raw_string_ostream ss(codestr);
	unsigned Line = SM.getSpellingLineNumber(SL);
	llvm::errs()<<"Line Number"<<SM.getSpellingLineNumber(SL)<<"\n";
	llvm::errs()<<"Column Number"<<SM.getSpellingColumnNumber(SL)<<"\n";
	auto FID = SM.getFileID(SL);
	clang::SourceRange SR(SM.translateLineCol(FID, Line, 1),
			      SM.translateLineCol(FID, Line,
						  std::numeric_limits<unsigned int>::max()));

	VD->print(ss);
	ss<<";\n";
	ss<<"/*add init*/"<<"\n";
	rew.ReplaceText(SR,  ss.str().c_str());
      }
      return true;
    }
    if(name.find("__xmp_align") == 0){
      auto content = llvm::dyn_cast<clang::InitListExpr>(vdecl->getInit());
      assert((content));
      auto align_stmt = llvm::dyn_cast<clang::UnaryOperator>
	(content->getInit(0)->IgnoreCasts());
      auto declref = llvm::dyn_cast<clang::DeclRefExpr>
	(align_stmt->getSubExpr());
      auto align_decl = declref->getDecl();
      align_decl->dump();
      rew.InsertTextBefore(vdecl->getBeginLoc(),
			   "/* Pragma Align Found */");
    }
    llvm::errs()<<vdecl->getName()<<"\n";
    return true;
  }
  /* 関数宣言を見つけると呼ばれるコールバック関数 */
  bool VisitFunctionDecl(clang::FunctionDecl *fdecl) {
    rew.InsertTextBefore(fdecl->getBeginLoc(),
                         "/* found during AST traversal */\n");
    return true;
  }
};
static void AddVoidPtr(clang::Preprocessor &PP,
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
static void AddVar(clang::Preprocessor &PP,
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
static void CreateUIntToken(clang::Preprocessor &PP,
			    clang::Token &Tok, unsigned int val,
			    clang::SourceLocation Start,
			    clang::SourceLocation End)
{
  std::string vstr = std::to_string(val);
  Tok.startToken();
  Tok.setKind(clang::tok::numeric_constant);
  PP.CreateString(vstr.c_str(), Tok, Start, End);
}
static void SetNumericConstant(clang::Token &Tok, const char *number)
{
  Tok.startToken();
  Tok.setKind(clang::tok::numeric_constant);
  Tok.setLiteralData(number);
  Tok.setLength(std::strlen(number));
}

static bool ArrayParser(clang::Preprocessor &PP,
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
		       
static void AddVoidCastToken(clang::SmallVector<clang::Token, 1> &TokenList,
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
static void AddTokenPtrElem(clang::SmallVector<clang::Token, 1> &TokenList,
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
static void AddEndBrace(clang::SmallVector<clang::Token, 1> &TokenList)
{
  clang::Token Tok;
  Tok.startToken();
  Tok.setKind(clang::tok::r_brace);
  TokenList.push_back(Tok);
  Tok.startToken();
  Tok.setKind(clang::tok::semi);
  TokenList.push_back(Tok);
}
struct numrange{
  int x;
  int y;
};
class PragmaNodesHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaNodesHandler():clang::PragmaHandler("nodes"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
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
};

class PragmaTemplateHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaTemplateHandler():clang::PragmaHandler("template"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
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
    
    name = std::string("__xmp_template") + std::to_string(nodes);
    nodes++;
    if(!ArrayParser(PP, arrays, Tok)){
      goto error;
    }
    //Discard to EOD manually.
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
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
};

class PragmaDistributeHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaDistributeHandler():clang::PragmaHandler("distribute"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    int numtoken = 0;
    clang::Token distTok;
    clang::Token nameTok;
    clang::Token Tok;
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::SmallVector<clang::Token,1>  Params;
    clang::tok::TokenKind expected;
    clang::Token zeroToken;
    clang::SourceLocation StartLoc = FirstTok.getLocation();    
    clang::IdentifierInfo *II;
    std::string idstr;
    std::string name;
    
    name = std::string("__xmp_distribute") + std::to_string(nodes);
    SetNumericConstant(zeroToken, "0");      
    PP.Lex(distTok);


    if(!distTok.is(expected = clang::tok::identifier)){
      goto error;
    }
    PP.Lex(Tok);
    if(!Tok.is((expected = clang::tok::l_square))){
      goto error;
    }
    while(Tok.is(clang::tok::l_square)){
      PP.Lex(Tok);
      if(Tok.is(clang::tok::star)){
	Params.push_back(zeroToken);
	Params.push_back(zeroToken);
	PP.Lex(Tok);
	if(!Tok.is(expected = clang::tok::r_square)){
	  goto error;
	}
	continue;
      } else  if(!Tok.is(expected = clang::tok::identifier)){
	goto error;
      }
      idstr = Tok.getIdentifierInfo()->getName().str();
      if(idstr == "block"){
	SetNumericConstant(Tok, "1");
	Params.push_back(Tok);
      }else if(idstr == "cyclic"){
	SetNumericConstant(Tok, "2");
	Params.push_back(Tok);
      }else if(idstr == "gblock"){
	SetNumericConstant(Tok, "3");
	Params.push_back(Tok);
      }else{
	goto error;
      }
      PP.Lex(Tok);
      if(Tok.is(clang::tok::l_paren)){
	PP.Lex(Tok);
	//if(!gblock)
	if(!Tok.is(expected = clang::tok::numeric_constant)){
	  goto error;
	}
	Params.push_back(Tok);
	PP.Lex(Tok);
	if(!Tok.is(expected = clang::tok::r_paren)){
	  goto error;
	}
      }else{
	Params.push_back(zeroToken);
      }
      if(!Tok.is(expected = clang::tok::r_square)){
	goto error;
      }
      PP.Lex(Tok);
    }
    
    if(!Tok.is(expected = clang::tok::identifier)){
      goto error;
    }
    idstr = Tok.getIdentifierInfo()->getName().str();
    if(idstr != "onto"){
      goto error;
    }
    PP.Lex(nameTok);
    if(!nameTok.is(expected = clang::tok::identifier)){
      goto error;
    }
    /*Discard tokens to eod*/
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
    /*Construct void * array*/
    {
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, distTok);
      AddTokenPtrElem(TokenList, nameTok);
      for(auto &&element : Params){
	AddVoidCastToken(TokenList, element);
	Tok.startToken();
	Tok.setKind(clang::tok::comma);
	TokenList.push_back(Tok);
      }
      AddEndBrace(TokenList);
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
};
class PragmaAlignHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaAlignHandler():clang::PragmaHandler("align"),nodes(0){}
  
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::Token alignTok;
    clang::Token tempTok;
    clang::Token Tok;
    clang::tok::TokenKind expected;
    clang::SmallVector<clang::Token,1>  TokenList;
    std::vector<int> arraykind;
    std::string idstr;
    std::string name;
    int arraydim = 0;
    int tempdim = 0;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    clang::SourceLocation EndLoc;
    name = std::string("__xmp_align") + std::to_string(nodes);    
    PP.Lex(alignTok);
    if(!alignTok.is(expected = clang::tok::identifier)){
      goto error;
    }
    PP.Lex(Tok);
    while(Tok.is(clang::tok::l_square)){
      arraydim ++;
      PP.Lex(Tok);
      if(Tok.is(clang::tok::star)){
	arraykind.push_back(1);
      }else if(Tok.is(clang::tok::colon)){
	arraykind.push_back(2);
      } else if(Tok.is(expected = clang::tok::identifier)){
	arraykind.push_back(0);
      }else{
	goto error;
      }
      PP.Lex(Tok);
      if(!Tok.is(expected = clang::tok::r_square)){
	goto error;
      }
      PP.Lex(Tok);
    }
    if(!Tok.is(expected = clang::tok::identifier)){
      goto error;
    }
    idstr = Tok.getIdentifierInfo()->getName().str();
    if(idstr != "with"){
      goto error;
    }
    PP.Lex(tempTok);
    if(!tempTok.is(expected = clang::tok::identifier)){
      goto error;
    }

    PP.Lex(Tok);
    while(Tok.is(clang::tok::l_square)){
      int offs = 0;
      PP.Lex(Tok);
      if(tempdim >=arraydim){
	if(!Tok.is(expected = clang::tok::star)){
	  goto error;
	}
	PP.Lex(Tok);
	if(!Tok.is(expected = clang::tok::r_square)){
	  goto error;
	}
      }else if(Tok.is(clang::tok::identifier) && arraykind[tempdim] == 0){
	PP.Lex(Tok);
	while(!Tok.is(clang::tok::r_square)){
	  //TODO: evaluate token to r_square
	  PP.Lex(Tok);
	}
	arraykind[tempdim] =  offs;
      }else if(Tok.is(clang::tok::colon)&& arraykind[tempdim] == 2){
	PP.Lex(Tok);
	if(!Tok.is(expected = clang::tok::r_square)){
	  goto error;
	}
	arraykind[tempdim] =  offs;
      }
      tempdim ++;
      PP.Lex(Tok);
    }

    /* check for excess element*/
    for(int i = tempdim; i < arraydim; i++){
      if(arraykind[i] != 1){
	expected = clang::tok::star;
	goto error;
      }
    }
    llvm::errs()<<arraydim<<","<<tempdim<<"\n";
    /*Discard tokens to eod*/
    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
    
    /*Construct void * array*/
    {
      AddVar(PP, TokenList, name, StartLoc);
      AddTokenPtrElem(TokenList, alignTok);
      AddTokenPtrElem(TokenList, tempTok);
      CreateUIntToken(PP, Tok, arraydim, StartLoc, EndLoc);
      AddVoidCastToken(TokenList, Tok);
      Tok.startToken();
      Tok.setKind(clang::tok::comma);
      TokenList.push_back(Tok);
      CreateUIntToken(PP, Tok, tempdim, StartLoc, EndLoc);
      AddVoidCastToken(TokenList, Tok);
      Tok.startToken();
      Tok.setKind(clang::tok::comma);
      TokenList.push_back(Tok);

      for(int i = 0; i < std::min(arraydim, tempdim) ; i++){
	CreateUIntToken(PP, Tok, arraykind[i], StartLoc, EndLoc);
	AddVoidCastToken(TokenList, Tok);
	Tok.startToken();
	Tok.setKind(clang::tok::comma);
	TokenList.push_back(Tok);
      }
      AddEndBrace(TokenList);
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
};
class PragmaShadowHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaShadowHandler():clang::PragmaHandler("shadow"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    /*array identifier, size for each dimension */
    PP.DiscardUntilEndOfDirective();
  };
};

class PragmaReflectHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaReflectHandler():clang::PragmaHandler("reflect"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    /*arrayname, pereidic?, width, orthognal? ,async id*/
    PP.DiscardUntilEndOfDirective();
  };
};
class PragmaTaskHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaTaskHandler():clang::PragmaHandler("task"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    std::string name = std::string("__xmp_task")+std::to_string(nodes);
    clang::tok::TokenKind expected;
    clang::Token nodeTok;
    std::string idstr;
    clang::Token Tok;
    nodes++;
    PP.Lex(Tok);

    if(!Tok.is(expected = clang::tok::identifier)){
      goto error;
    }
    idstr = Tok.getIdentifierInfo()->getName().str();
    if(idstr != "on"){
      goto error;
    }
    PP.Lex(nodeTok);
    if(!nodeTok.is(expected = clang::tok::identifier)){
      goto error;
    }

    while(!Tok.is(clang::tok::eod)){
      PP.Lex(Tok);
    }
    {
      AddVar(PP, TokenList, name, StartLoc);
      AddVoidCastToken(TokenList, nodeTok);
      AddEndBrace(TokenList);
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
  };
};
class PragmaTasksHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaTasksHandler():clang::PragmaHandler("tasks"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::SourceLocation StartLoc = FirstTok.getLocation();
    std::string name = std::string("__xmp_tasks")+std::to_string(nodes);

    nodes++;
    AddVar(PP, TokenList, name, StartLoc);
    AddEndBrace(TokenList);
    PP.DiscardUntilEndOfDirective();
    auto TokenArray = std::make_unique<clang::Token[]>(TokenList.size());
    std::copy(TokenList.begin(), TokenList.end(), TokenArray.get());
    PP.EnterTokenStream(std::move(TokenArray), TokenList.size(),
			/*DisableMacroExpansion=*/false,
			/*IsReinject=*/false);
  };
};

class PragmaWaitAsyncHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaWaitAsyncHandler():clang::PragmaHandler("wait_async"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    /*Asyncid, node or template ref*/
    PP.DiscardUntilEndOfDirective();
  };
};

class PragmaGmoveHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaGmoveHandler():clang::PragmaHandler("gmove"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    /*in|out|corrective, async id*/
    PP.DiscardUntilEndOfDirective();
    /*Parse next statement should be parsed*/
  };
};

class PragmaBcastHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaBcastHandler():clang::PragmaHandler("bcast"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    /*variables srcnoderef, destnoderef*/
    PP.DiscardUntilEndOfDirective();
  };
};


class PragmaReductionHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaReductionHandler():clang::PragmaHandler("reduction"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    /*operand variable, noderef, async id.*/
    PP.DiscardUntilEndOfDirective();
  };
};
class PragmaBarrierHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaBarrierHandler():clang::PragmaHandler("barrier"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    /*node or template ref*/
    PP.DiscardUntilEndOfDirective();
  };

};

class PragmaReduceShadowHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaReduceShadowHandler():clang::PragmaHandler("reduce_shadow"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    clang::Token Tok;
    /*var width, orthognal, asyncid*/
    PP.Lex(Tok);
    if(!Tok.is(clang::tok::identifier)){
      goto error;
    }

  error:
    PP.DiscardUntilEndOfDirective();
  };

};

class PragmaTemplateFixHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaTemplateFixHandler():clang::PragmaHandler("template_fix"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    /* template name, count of element ,dist format*/
    PP.DiscardUntilEndOfDirective();
  };

};

class PragmaArrayHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaArrayHandler():clang::PragmaHandler("array"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
    clang::SmallVector<clang::Token,1>  TokenList;
    /* Template ref and process*/
    PP.DiscardUntilEndOfDirective();
    /*
     *Process Next Data Assignment statements,which use extended syntax
     * That cannot be parsed by standard C parser.
     */
  };

};


class PragmaLoopHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaLoopHandler():clang::PragmaHandler("loop"),nodes(0){}
  
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override{
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
      /**/
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
};

/* ASTを受取るクラス */
class MyASTConsumer : public clang::ASTConsumer {
  clang::CompilerInstance *cmpi;

public:
  MyASTConsumer(clang::CompilerInstance *ci) : cmpi(ci) {
    auto &PP = ci->getPreprocessor();
    PP.AddPragmaHandler("xmp", new PragmaNodesHandler());
    PP.AddPragmaHandler("xmp", new PragmaTemplateHandler());
    PP.AddPragmaHandler("xmp", new PragmaDistributeHandler());
    PP.AddPragmaHandler("xmp", new PragmaAlignHandler());
    PP.AddPragmaHandler("xmp", new PragmaArrayHandler());
    PP.AddPragmaHandler("xmp", new PragmaLoopHandler());
    PP.AddPragmaHandler("xmp", new PragmaShadowHandler());
    PP.AddPragmaHandler("xmp", new PragmaReflectHandler());
    PP.AddPragmaHandler("xmp", new PragmaTaskHandler());
    PP.AddPragmaHandler("xmp", new PragmaTasksHandler());
    PP.AddPragmaHandler("xmp", new PragmaGmoveHandler());
    PP.AddPragmaHandler("xmp", new PragmaBcastHandler());
    PP.AddPragmaHandler("xmp", new PragmaWaitAsyncHandler());
    PP.AddPragmaHandler("xmp", new PragmaReductionHandler());
    PP.AddPragmaHandler("xmp", new PragmaBarrierHandler());
    PP.AddPragmaHandler("xmp", new PragmaReduceShadowHandler());
    PP.AddPragmaHandler("xmp", new PragmaTemplateFixHandler());
  }

  /* コンパイル対象のファイルごとに呼ばれる関数 */
  virtual void HandleTranslationUnit(clang::ASTContext &ast) {
    /* コード書き換えクラスの定義と設定 */
    clang::Rewriter rew;
    rew.setSourceMgr(cmpi->getSourceManager(), cmpi->getLangOpts());
    for (clang::Decl *D : ast.getTraversalScope()){
      D->dump();
    }
    /* ASTを探索するクラス */
    MyASTVisitor visitor(rew, ast);
    visitor.TraverseAST(ast); /* 探索開始 */

    /* 対象ファイルのID取得*/
    clang::FileID id = cmpi->getSourceManager().getMainFileID();
    /* 書き換え結果の取得 (ASTVisitorが書き換えたこと前提) */
    const clang::RewriteBuffer *rbuf = rew.getRewriteBufferFor(id);
    if (rbuf != nullptr) {
      std::string buf(rbuf->begin(), rbuf->end());
      std::cerr << buf;
    } else {
      std::cerr << "Original code is unchanged" << std::endl;
    }
  }
};

/* コンパイラのインスタンス */
class MyFrontendAction : public clang::SyntaxOnlyAction {
public:
  /* ASTを受取るクラスの生成 */
  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &ci, llvm::StringRef file) {
    return std::make_unique<MyASTConsumer>(&ci);
  }
};

static llvm::cl::OptionCategory MinicOptionCategory("Mini-c Options");
static llvm::cl::extrahelp CommonHelp("hoge\n");
static llvm::cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char **argv) {
  clang::tooling::CommonOptionsParser op(argc, argv, MinicOptionCategory);

  clang::tooling::ClangTool tool(op.getCompilations(), op.getSourcePathList());

#if 1
  /* コンパイラ起動時のオプション設定 */
  tool.clearArgumentsAdjusters();
  tool.appendArgumentsAdjuster(clang::tooling::getClangSyntaxOnlyAdjuster());
  tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(
      "-Wno-unused-value", clang::tooling::ArgumentInsertPosition::BEGIN));
  // avoid including g++ header files
#ifdef XEV_INCLUDE_PATH
  /* GNU Cがインストールされているとエラーが出るのでおまじない */
  tool.appendArgumentsAdjuster(clang::tooling::getInsertArgumentAdjuster(
      XEV_INCLUDE_PATH, clang::tooling::ArgumentInsertPosition::BEGIN));
#endif
#endif

  /* FrontendActionクラスの派生形に合わせてコンパイラ起動 */
  std::unique_ptr<clang::tooling::FrontendActionFactory> FrontendFactory;
  FrontendFactory =
      clang::tooling::newFrontendActionFactory<MyFrontendAction>();
  return tool.run(FrontendFactory.get());
}
