#include "SingleReturnCheck.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang-tidy/ClangTidyCheck.h"

using namespace clang::ast_matchers;

namespace clang::tidy::custom {

SingleReturnCheck::SingleReturnCheck(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context) {}

void SingleReturnCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      functionDecl(unless(isMain())).bind("func"), this);
}

void SingleReturnCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *Func = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (!Func || !Func->hasBody()) return;

  const auto *Body = dyn_cast<CompoundStmt>(Func->getBody());
  if (!Body) return;

  unsigned ReturnCount = 0;
  for (const auto *Stmt : Body->body()) {
    if (isa<ReturnStmt>(Stmt)) {
      ReturnCount++;
      if (ReturnCount > 1) {
        diag(Stmt->getBeginLoc(), "LGEDV-01: Function '%0' has multiple return points")
            << Func->getName();
      }
    }
  }
}

} // namespace clang::tidy::custom