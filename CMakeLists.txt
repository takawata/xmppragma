cmake_minimum_required(VERSION 2.8.4)
project(xmpplagma)
set(default_build_type "Debug")
set(_IMPORT_PREFIX "/usr/local/llvm90")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG
  HINTS ${LLVM_INSTALL_PREFIX}/lib/cmake/clang
)
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

set(CLANG_VERSION ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}.${LLVM_VERSION_PATCH})
set(EXTRA_INCLUDE_DIR ${CLANG_INSTALL_PREFIX}/lib/clang/${CLANG_VERSION}/include)
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

message(STATUS "Adding ${EXTRA_INCLUDE_DIR} to the include search paths.")

add_definitions(
-D__STDC_LIMIT_MACROS
-D__STDC_CONSTANT_MACROS
-DXEV_INCLUDE_PATH="-I${EXTRA_INCLUDE_DIR}"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES 
  src/main.cpp
  src/Rewriter.cpp
  src/RewriteUtil.cpp
  src/AlignHandler.cpp
  src/NodeHandler.cpp
  src/PPUtil.cpp
  src/PPAlign.cpp
  src/PPArray.cpp
  src/PPBarrier.cpp
  src/PPBcast.cpp
  src/PPDistribute.cpp
  src/PPGmove.cpp
  src/PPLoop.cpp
  src/PPNode.cpp
  src/PPReflect.cpp
  src/PPReduction.cpp
  src/PPShadow.cpp
  src/PPTask.cpp
  src/PPTasks.cpp
  src/PPTemplate.cpp
  src/PPTemplateFix.cpp
  src/PPReduceShadow.cpp
  src/PPWaitAsync.cpp
 )

add_executable(xmpplagma ${SOURCE_FILES})

target_link_libraries(xmpplagma
  clangTooling
  clangFrontendTool
  clangFrontend
  clangDriver
  clangSerialization
  clangCodeGen
  clangParse
  clangSema
  clangStaticAnalyzerFrontend
  clangStaticAnalyzerCheckers
  clangStaticAnalyzerCore
  clangAnalysis
  clangARCMigrate
  clangRewrite 
  clangRewriteFrontend
  clangEdit 
  clangAST 
  clangLex 
  clangBasic 
  clang
)

target_link_libraries(xmpplagma
 LLVM
)
