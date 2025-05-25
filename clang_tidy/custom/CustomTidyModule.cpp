#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"
#include "ClangTidyModuleRegistry.h"
#include "SingleReturnCheck.h"
//#include "NoNewCheck.h"
//#include "NoGlobalVarCheck.h"
//#include "OverrideCheck.h"
//#include "NoGotoCheck.h"

namespace clang::tidy::custom {

class CustomModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<SingleReturnCheck>("custom-SingleReturnCheck");
    //CheckFactories.registerCheck<NoNewCheck>("custom-NoNewCheck");
    //CheckFactories.registerCheck<NoGlobalVarCheck>("custom-NoGlobalVarCheck");
    //CheckFactories.registerCheck<OverrideCheck>("custom-OverrideCheck");
    //CheckFactories.registerCheck<NoGotoCheck>("custom-NoGotoCheck");
  }
};

} // namespace clang::tidy::custom

namespace clang::tidy {
static ClangTidyModuleRegistry::Add<clang::tidy::custom::CustomModule>
    X("custom-module", "Adds custom lint checks.");
}