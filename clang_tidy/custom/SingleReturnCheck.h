#ifndef CLANG_TIDY_CUSTOM_SINGLERETURNCHECK_H
#define CLANG_TIDY_CUSTOM_SINGLERETURNCHECK_H

#include "clang-tidy/ClangTidyCheck.h"
#include "clang/ASTMatchers/ASTMatchers.h"

namespace clang::tidy::custom {

class SingleReturnCheck : public ClangTidyCheck {
public:
  SingleReturnCheck(StringRef Name, ClangTidyContext *Context);
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
};

} // namespace clang::tidy::custom

#endif // CLANG_TIDY_CUSTOM_SINGLERETURNCHECK_H