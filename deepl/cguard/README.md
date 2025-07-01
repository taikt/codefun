# installation
sudo apt-get update
sudo su
apt-get install -y ninja-build build-essential cmake clang git python3 libstdc++-12-dev zlib1g-dev cppcheck clang && sudo apt-get install npm && sudo npm install -g vsce && sudo npm install typescript -g

# check port
sudo netstat -tulpn | grep :8080

# start server
python3 optimized_viewer.py

# build
npm run compile
vsce package
code --install-extension lgedv-clean-1.0.0.vsix
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


# prompt template
Check for C++ rule violation based on the guidelines in Misracpp2008Guidelines_en.md. If the guidelines file is not existed, please download the file by calling fetch_misra_rule from MCP server. 
Please scan full file of the code, and check every rule violation detailed.
Explain rule content if a rule is violated and show code line number of code that violates the rule.
If a rule is not violated, please do not mention it.
Suggest refactored code to comply with these rules.