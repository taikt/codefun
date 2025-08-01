# Install packages
sudo apt-get update
sudo apt-get install -y ninja-build build-essential cmake clang git python3 libstdc++-12-dev zlib1g-dev cppcheck clang && sudo apt-get install npm && sudo npm install -g vsce && sudo npm install typescript -g && apt-get install graphviz

## for static analysic tool
### install clang
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh 
sudo ./llvm.sh 20
sudo apt-get install libclang-20-dev clang-20
ls /usr/lib/llvm-20/lib/libclang.so

# build extension
npm run compile
vsce package
vsce ls   (=> to show what is packaged)
code --install-extension lgedv-codeguard-1.0.5.vsix
-> reload vscode

(neu bi loi khi vsce package, nang cap version)
# Nếu chưa có nvm, cài nvm:
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
source ~/.bashrc
# Cài Node.js mới nhất (hoặc đúng bản yêu cầu)
nvm install 20.18.1
nvm use 20.18.1
# Kiểm tra lại
node -v
npm -v


# Upload extension
https://marketplace.visualstudio.com/manage/publishers/trananhtai-lge
(username: tai2.tran@lge.com, pw: lg's login)
token:
https://dev.azure.com/tai2tran/_usersSettings/tokens
2fnQLq4OeeYMx0lk6HFMsKFxyaBcooZyKPkny1eaWcIZf9lFMFLKJQQJ99BGACAAAAArP9ilAAASAZDOvwko

vsce publish

(neu token bi het han)
1. Truy cập link tạo token mới:
   - https://dev.azure.com/tai2tran/_usersSettings/tokens
   
2. Tạo token mới với quyền "All accessible organizations" hoặc ít nhất là "Marketplace (Publish)".
vsce login TranAnhTai-LGE
vsce publish

# start web server manually
cd d:\src\codefun\deepl\cguard\src\web_srv
python -m venv venv
.\venv\Scripts\Activate.ps1
python server_aiohttp.py

# kiem tra status web server
## win:
netstat -ano | findstr :8888
curl http://localhost:8888/api/health
powershell -ExecutionPolicy Bypass -File "start_server.ps1" status



# Note 
8.1.2025
- Temporaly disable codeguard menu:
in package.json, doi ten  (de vscode khong nhan ra keyword)
menus -> menus_disabled
submenus -> submenus_disabled

- diable commands cho extension (ctr+shift+P), chi giu lai Install MCP
in package.json: doi commands -> commands_disabled
tao commands moi chi chua "install MCP"
