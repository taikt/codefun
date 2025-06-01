#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"
#include "SingleReturnCheck.h"
#include "SwitchBreakCheck.h"
#include <iostream>

namespace clang::tidy {
namespace custom {
class CustomModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<SingleReturnCheck>("custom-SingleReturnCheck");
    CheckFactories.registerCheck<SwitchBreakCheck>("custom-SwitchBreakCheck");
  }
};
}

static ClangTidyModuleRegistry::Add<custom::CustomModule> X("custom-module", "Adds custom lint checks for LGEDV rules.");

volatile int CustomModuleAnchorSource = 0;

} // namespace clang::tidy::custom