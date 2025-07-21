
# install packages
sudo apt-get update
sudo su
apt-get install -y ninja-build build-essential cmake clang git python3 libstdc++-12-dev zlib1g-dev cppcheck clang && sudo apt-get install npm && sudo npm install -g vsce && sudo npm install typescript -g && apt-get install graphviz


# build
npm run compile
vsce package
code --install-extension lgedv-codeguard-1.0.0.vsix
-> reload vscode

# upload
https://marketplace.visualstudio.com/manage/publishers/taikt123
(username: tai2.tran@lge.com, pw: lg's login)
token:
https://dev.azure.com/tai2tran/_usersSettings/tokens
2fnQLq4OeeYMx0lk6HFMsKFxyaBcooZyKPkny1eaWcIZf9lFMFLKJQQJ99BGACAAAAArP9ilAAASAZDOvwko

vsce publish

(neu token bi het han)
1. Truy cập link tạo token mới:
   - https://dev.azure.com/tai2tran/_usersSettings/tokens
   
2. Tạo token mới với quyền "All accessible organizations" hoặc ít nhất là "Marketplace (Publish)".
  
3. Đăng nhập lại với token mới:
   ```bash
   vsce login <publisher-name>
   vd: vsce login TranAnhTai-LGE
   ```
   - Khi được hỏi, dán token mới vào.

4. Thực hiện lại lệnh publish:
   ```bash
   vsce publish
   ```

**Lưu ý:**
- Nên bổ sung trường `"repository"` và file `LICENSE` vào `package.json` để tránh cảnh báo khi publish.


Sau khi dùng token mới, bạn sẽ publish extension thành công!