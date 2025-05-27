#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"
#include "SingleReturnCheck.h"
#include <iostream>

namespace clang::tidy {
namespace custom {
class CustomModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    std::cerr << "CustomModule: Registering SingleReturnCheck\n";
    CheckFactories.registerCheck<SingleReturnCheck>("custom-SingleReturnCheck");
  }
};
}

static ClangTidyModuleRegistry::Add<custom::CustomModule> X("custom-module", "Adds custom lint checks for LGEDV rules.");

volatile int CustomModuleAnchorSource = 0;


} // namespace clang::tidy::custom