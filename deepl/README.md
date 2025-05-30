this is deep learning space

# clang-tidy custom check
apt-get update && apt-get install -y ninja-build build-essential cmake clang git python3 libstdc++-12-dev zlib1g-dev

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release   -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra"   -DLLVM_TARGETS_TO_BUILD="X86"   -DCMAKE_CXX_FLAGS="-O2" ../llvm
ninja -j4
./bin/clang-tidy --list-checks
./bin/clang-tidy ../src/main.cpp -p ../build-test
./bin/clang-tidy --checks=-*,custom-SingleReturnCheck ./src/main.cpp -p ../build-test
./bin/clang-tidy --checks=-*,custom-SwitchBreakCheck ./../clang-tools-extra/clang-tidy/custom/test.cpp


build vscode extension
PS C:\Users\tai2.tran\lgedv-checker\lgedv-checker> npm run compile
PS C:\Users\tai2.tran\lgedv-checker\lgedv-checker> vsce package
PS C:\Users\tai2.tran\lgedv-checker\lgedv-checker> vsce publish
PS C:\Users\tai2.tran\lgedv-checker\lgedv-checker> scp lgedv-checker-0.0.2.vsix worker@tai2-tran-tsu-my26-taikt.vbee.lge.com:/home/worker/

sudo cp /home/worker/src/llvm-project/build/bin/clang-tidy /usr/local/bin/
clang-tidy --checks=-*,custom-* DAQApplication.cpp

# svg icon 
<svg width="128" height="128" xmlns="http://www.w3.org/2000/svg">
  <circle cx="64" cy="64" r="56" fill="#E91E63"/>
  <text x="50%" y="50%" font-size="10" font-family="Arial, Helvetica, sans-serif" fill="#FFFFFF" font-weight="bold" text-anchor="middle" dominant-baseline="central">
    LGEDV-CodeChecker
  </text>
</svg>