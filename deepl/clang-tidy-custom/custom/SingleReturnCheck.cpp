#include "SingleReturnCheck.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang-tidy/ClangTidyCheck.h"

using namespace clang::ast_matchers;

namespace clang::tidy::custom {

SingleReturnCheck::SingleReturnCheck(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context) {}

void SingleReturnCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(functionDecl(unless(isMain())).bind("func"), this);
}

void SingleReturnCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (!Func || !Func->hasBody()) return;

  // Đếm ReturnStmt bằng RecursiveASTVisitor
  unsigned ReturnCount = 0;
  auto Visitor = [this, &ReturnCount, Func](const Stmt *S) {
    if (const auto *Return = dyn_cast<ReturnStmt>(S)) {
      ReturnCount++;
      if (ReturnCount > 1) {
        diag(Return->getBeginLoc(), "LGEDV_CRCL_0003: Function '%0' has multiple return points")
            << Func->getName();
      }
    }
  };

  class ReturnVisitor : public RecursiveASTVisitor<ReturnVisitor> {
  public:
    explicit ReturnVisitor(std::function<void(const Stmt *)> V) : Visitor(V) {}
    bool VisitStmt(Stmt *S) {
      Visitor(S);
      return true;
    }
  private:
    std::function<void(const Stmt *)> Visitor;
  };

  ReturnVisitor VisitorFunc(Visitor);
  VisitorFunc.TraverseStmt(Func->getBody());
}

} // namespace clang::tidy::custom