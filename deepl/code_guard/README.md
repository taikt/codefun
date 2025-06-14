# tools
https://www.mermaidchart.com/app/dashboard
https://www.svg2png.com/

# install clang-tidy, cppcheck, clang, npm, typescript
sudo apt-get update && apt-get install -y ninja-build build-essential cmake clang git python3 libstdc++-12-dev zlib1g-dev cppcheck clang && sudo apt-get install npm && sudo npm install -g vsce && sudo npm install typescript -g


# build clang-tidy 
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release   -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra"   -DLLVM_TARGETS_TO_BUILD="X86"   -DCMAKE_CXX_FLAGS="-O2" ../llvm

ninja -j4

./bin/clang-tidy --list-checks
./bin/clang-tidy ../src/main.cpp -p ../build-test
./bin/clang-tidy --checks=-*,custom-SingleReturnCheck ./src/main.cpp -p ../build-test
./bin/clang-tidy --checks=-*,custom-SwitchBreakCheck ./../clang-tools-extra/clang-tidy/custom/test.cpp

sudo cp /home/worker/src/llvm-project/build/bin/clang-tidy /usr/local/bin/
clang-tidy --checks=-*,custom-* DAQApplication.cpp

# build vscode extension
npm run compile
vsce package

vsce publish
# https://marketplace.visualstudio.com/manage/publishers/taikt123
# token:
# https://dev.azure.com/tai2tran/_usersSettings/tokens
FIhyEDH4hyvE8B26bKwYc7brGiZwUQjyY4qwLR2TM9p8WpQkY8QtJQQJ99BFACAAAAArP9ilAAASAZDO1UGl
account: tai2.tran@lge.com, LGE PC pw
publisher: taikt123

# delete publisher: 
vsce delete-publisher publisher_id

# change cmake, python version on vbee containter (optional)
sudo unlink /usr/bin/cmake
sudo ln -s ~/cmake-3.22.6/bin/cmake /usr/bin/cmake
sudo apt install -y ninja-build

cd ~/Python-3.9.18
sudo make install
sudo unlink /usr/bin/python
sudo ln -s /usr/local/bin/python3 /usr/bin/python
cd --

# Install Ollama
sudo curl -fsSL https://ollama.com/install.sh | sh

# Download open-source model
ollama pull qwen3:8b
ollama serve &
# Run a model
ollama run qwen3:8b 

# install virtual env
sudo apt update
sudo apt install python3
python3 -m venv ~/src/dl/ollama_env

source /home/worker/src/dl/ollama_env/bin/activate
pip install ollama



