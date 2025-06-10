#include "SwitchBreakCheck.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/AST/Stmt.h"
#include "clang-tidy/ClangTidyCheck.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang::ast_matchers;

namespace clang::tidy::custom {

// Đệ quy kiểm tra có break trong statement
static bool hasBreakInStmts(const std::vector<const clang::Stmt*>& stmts) {
  for (const auto* S : stmts) {
    if (!S) continue;
    if (llvm::isa<clang::BreakStmt>(S)) return true;
    if (const auto* CS = llvm::dyn_cast<clang::CompoundStmt>(S)) {
      if (hasBreakInStmts(std::vector<const clang::Stmt*>(CS->body_begin(), CS->body_end())))
        return true;
    }
    for (const auto* Child : S->children()) {
      if (hasBreakInStmts({Child})) return true;
    }
  }
  return false;
}

SwitchBreakCheck::SwitchBreakCheck(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context) {}

void SwitchBreakCheck::registerMatchers(ast_matchers::MatchFinder *Finder) {
  Finder->addMatcher(switchStmt().bind("switch"), this);
}

void SwitchBreakCheck::check(const ast_matchers::MatchFinder::MatchResult &Result) {
  const auto *Switch = Result.Nodes.getNodeAs<clang::SwitchStmt>("switch");
  if (!Switch || !Switch->getBody()) return;
  if (Result.SourceManager->isInSystemHeader(Switch->getBeginLoc())) return;

  const auto *Body = llvm::dyn_cast<clang::CompoundStmt>(Switch->getBody());
  if (!Body) return;

  // Chuyển sang vector để truy cập chỉ số
  std::vector<const clang::Stmt*> stmts;
  for (const auto *S : Body->body()) {
    stmts.push_back(S);
  }
  std::vector<size_t> caseIndices;
  for (size_t i = 0; i < stmts.size(); ++i) {
    if (llvm::isa<clang::SwitchCase>(stmts[i]))
      caseIndices.push_back(i);
  }
  caseIndices.push_back(stmts.size()); // Để xử lý case cuối cùng

  // Duyệt từng case, kiểm tra break trong child của case label nếu có, nếu không thì kiểm tra các statement sau case label
  for (size_t c = 0; c + 1 < caseIndices.size(); ++c) {
    size_t begin = caseIndices[c];
    size_t end = caseIndices[c + 1];
    const auto *CurCase = llvm::dyn_cast<clang::SwitchCase>(stmts[begin]);
    const clang::Stmt* sub = CurCase ? CurCase->getSubStmt() : nullptr;

    // DEBUG: In thông tin từng case
    // llvm::errs() << "[DEBUG] Case at line: "
    //              << Result.SourceManager->getSpellingLineNumber(CurCase->getBeginLoc())
    //              << ", begin=" << begin << ", end=" << end
    //              << ", has sub: " << (sub ? "yes" : "no") << "\n";

    // Nếu sub là case label tiếp theo hoặc không có sub, kiểm tra các statement sau case label
    if (!sub || llvm::isa<clang::SwitchCase>(sub)) {
      std::vector<const clang::Stmt*> rangeStmts;
      for (size_t i = begin + 1; i < end; ++i) {
        rangeStmts.push_back(stmts[i]);
      }
      // llvm::errs() << "[DEBUG]   rangeStmts.size() = " << rangeStmts.size()
      //              << ", hasBreak = " << (hasBreakInStmts(rangeStmts) ? "yes" : "no") << "\n";
      // Chỉ báo lỗi nếu thực sự có statement thực thi mà không có break
      if (!rangeStmts.empty() && !hasBreakInStmts(rangeStmts)) {
        diag(CurCase->getBeginLoc(),
             "LGEDV_CRCL_0013: Each case in switch statement must end with a break statement");
      }
      continue;
    }

    // Nếu sub là block, kiểm tra break trong block đó
    if (const auto* CS = llvm::dyn_cast<clang::CompoundStmt>(sub)) {
      // llvm::errs() << "[DEBUG]   sub is block, hasBreak = "
      //              << (hasBreakInStmts(std::vector<const clang::Stmt*>(CS->body_begin(), CS->body_end())) ? "yes" : "no") << "\n";
      if (!hasBreakInStmts(std::vector<const clang::Stmt*>(CS->body_begin(), CS->body_end()))) {
        diag(CurCase->getBeginLoc(),
             "LGEDV_CRCL_0013: Each case in switch statement must end with a break statement");
      }
      continue;
    }

    // Nếu sub không phải block, kiểm tra break trong sub và các statement tiếp theo
    std::vector<const clang::Stmt*> rangeStmts;
    rangeStmts.push_back(sub);
    for (size_t i = begin + 1; i < end; ++i) {
      rangeStmts.push_back(stmts[i]);
    }
    // llvm::errs() << "[DEBUG]   sub is not block, rangeStmts.size() = " << rangeStmts.size()
    //              << ", hasBreak = " << (hasBreakInStmts(rangeStmts) ? "yes" : "no") << "\n";
    if (!rangeStmts.empty() && !hasBreakInStmts(rangeStmts)) {
      diag(CurCase->getBeginLoc(),
           "LGEDV_CRCL_0013: Each case in switch statement must end with a break statement");
    }
  }
}

} // namespace clang::tidy::custom