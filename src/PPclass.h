
class PragmaNodesHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaNodesHandler():clang::PragmaHandler("nodes"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaAlignHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaAlignHandler():clang::PragmaHandler("align"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaBcastHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaBcastHandler():clang::PragmaHandler("bcast"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaGmoveHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaGmoveHandler():clang::PragmaHandler("gmove"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};


class PragmaDistributeHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaDistributeHandler():clang::PragmaHandler("distribute"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaTaskHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaTaskHandler():clang::PragmaHandler("task"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaTasksHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaTasksHandler():clang::PragmaHandler("tasks"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaReflectHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaReflectHandler():clang::PragmaHandler("reflect"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaShadowHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaShadowHandler():clang::PragmaHandler("shadow"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};
class PragmaTemplateHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaTemplateHandler():clang::PragmaHandler("template"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaReductionHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaReductionHandler():clang::PragmaHandler("reduction"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaBarrierHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaBarrierHandler():clang::PragmaHandler("barrier"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaArrayHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaArrayHandler():clang::PragmaHandler("array"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaTemplateFixHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaTemplateFixHandler():clang::PragmaHandler("template_fix"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};



class PragmaLoopHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaLoopHandler():clang::PragmaHandler("loop"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaWaitAsyncHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaWaitAsyncHandler():clang::PragmaHandler("wait_async"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};

class PragmaReduceShadowHandler: public clang::PragmaHandler{
  int nodes;
public:
  PragmaReduceShadowHandler():clang::PragmaHandler("reduce_shadow"),nodes(0){}
  void HandlePragma(clang::Preprocessor &PP,
		    clang::PragmaIntroducer Introducer,
		    clang::Token &FirstTok) override;
};
