#ifndef CLANG_TIDY_CUSTOM_SWITCHBREAKCHECK_H
#define CLANG_TIDY_CUSTOM_SWITCHBREAKCHECK_H

#include "clang-tidy/ClangTidyCheck.h"
#include "clang/ASTMatchers/ASTMatchers.h"

namespace clang::tidy::custom {

class SwitchBreakCheck : public ClangTidyCheck {
public:
  SwitchBreakCheck(StringRef Name, ClangTidyContext *Context);
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace clang::tidy::custom

#endif // CLANG_TIDY_CUSTOM_SWITCHBREAKCHECK_H