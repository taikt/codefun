# TODO
1. evaluate accuracy
2. evaluate false positives

# Installation guide
sudo apt-get update
sudo su
apt-get install -y ninja-build build-essential cmake clang git python3 libstdc++-12-dev zlib1g-dev cppcheck clang && sudo apt-get install npm && sudo npm install -g vsce && sudo npm install typescript -g
&& apt-get install graphviz

# check port
sudo netstat -tulpn | grep :8080

# start server
python3 optimized_viewer.py

# build
npm run compile
vsce package
code --install-extension lgedv-codeguard-1.0.0.vsix
-> reload vscode

# "check LGEDV Rules" flow
Khi bạn click "Check LGEDV Rules" trong extension này, luồng hoạt động như sau:

1. Lệnh `lgedv.checkLGEDVRules` được đăng ký trong extension.ts:
```typescript
let checkLGEDVCommand = vscode.commands.registerCommand('lgedv.checkLGEDVRules', async () => {
  await runRuleAnalysis('LGEDV');
});
```

2. Khi lệnh này được gọi (bạn click), hàm `runRuleAnalysis('LGEDV')` sẽ chạy:
- Lấy nội dung file đang mở, kiểm tra loại file.
- Tạo output channel để log tiến trình.
- Đọc nội dung code, lấy timestamp.

3. Hàm `runRuleAnalysis`:
- Kết nối tới Copilot qua API `vscode.lm.selectChatModels({ vendor: 'copilot' })`.
- Đọc file rule LGEDV (LGEDVRuleGuide.md).
- Tạo prompt đặc biệt (có code, rule, hướng dẫn format output) bằng hàm `createFocusedPrompt`.
- Gửi prompt này tới Copilot model đã chọn bằng `model.sendRequest(messages, {}, cancellationToken.token)`.

4. Copilot trả về kết quả phân tích (dạng streaming), extension thu thập và hiển thị lên output channel.

5. Sau khi xong, extension lưu lại kết quả (Markdown + HTML report).

Tóm lại: Khi bạn click, extension sẽ lấy code + rule, tạo prompt, gửi tới Copilot qua API, nhận kết quả và hiển thị/lưu lại. Prompt luôn chứa code, rule, hướng dẫn format, và yêu cầu Copilot trả về đúng format để extension dễ xử lý.

Đoạn code lấy nội dung file đang mở, kiểm tra loại file, và đọc nội dung code trong extension.ts nằm trong hàm runRuleAnalysis:

```typescript
async function runRuleAnalysis(ruleType: 'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL') {
  const editor = vscode.window.activeTextEditor;
  if (!editor) {
    vscode.window.showErrorMessage('No active editor found. Please open a C++ file.');
    return;
  }

  if (editor.document.languageId !== 'cpp') {
    vscode.window.showErrorMessage('This extension only works with C++ files (.cpp, .h, .hpp)');
    return;
  }

  // Lấy đường dẫn file đang mở
  const filePath = editor.document.uri.fsPath;
  // Đọc toàn bộ nội dung code của file đang mở
  const codeContent = editor.document.getText();
  // ... các bước tiếp theo ...
}
```

Tóm lại:
- `const editor = vscode.window.activeTextEditor;` lấy editor hiện tại.
- `editor.document.languageId !== 'cpp'` kiểm tra loại file.
- `const filePath = editor.document.uri.fsPath;` lấy đường dẫn file.
- `const codeContent = editor.document.getText();` đọc toàn bộ nội dung code.


## loggo design
https://www.svgviewer.dev/svg-to-png

<svg width="256" height="256" viewBox="0 0 256 256" fill="none" xmlns="http://www.w3.org/2000/svg">
  <rect width="256" height="256" rx="56" fill="#667eea"/>
  <g>
    <path d="M128 44L208 76V128C208 176 128 212 128 212C128 212 48 176 48 128V76L128 44Z" fill="#fff" stroke="#2c3e50" stroke-width="8"/>
    <path d="M96 120L112 136L160 88" stroke="#667eea" stroke-width="10" stroke-linecap="round" stroke-linejoin="round"/>
  </g>
  <text x="128" y="242" text-anchor="middle" fill="#2c3e50" font-size="32" font-family="Segoe UI, Arial, sans-serif" font-weight="bold">CodeGuard</text>