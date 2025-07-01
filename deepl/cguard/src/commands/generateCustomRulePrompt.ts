import * as vscode from 'vscode';
import * as fs from 'fs';
import * as path from 'path';

export function registerGenerateCustomRulePromptCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.generateCustomRulePrompt', async () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
      vscode.window.showInformationMessage('No active editor');
      return;
    }
    const document = editor.document;
    const filePath = document.uri.fsPath;
    if (!filePath.endsWith('.cpp') && !filePath.endsWith('.cc')) {
      vscode.window.showInformationMessage('Selected file is not a C++ file');
      return;
    }
    // Đọc custom rule file path từ user settings
    const config = vscode.workspace.getConfiguration('lgedvCodeGuard');
    let customRuleFile = config.get<string>('customRuleFile') || '';
    if (!customRuleFile.trim()) {
      vscode.window.showWarningMessage(
        'Custom rule file path is empty. Please create a markdown rule file and set the path in settings (lgedvCodeGuard.customRuleFile).'
      );
      return;
    }
    // Resolve path nếu có ${workspaceFolder}
    if (customRuleFile.includes('${workspaceFolder}')) {
      const wsFolder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath || '';
      customRuleFile = customRuleFile.replace('${workspaceFolder}', wsFolder);
    }
    if (!path.isAbsolute(customRuleFile)) {
      const wsFolder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;
      if (wsFolder) {
        customRuleFile = path.join(wsFolder, customRuleFile);
      } else {
        customRuleFile = path.resolve(customRuleFile);
      }
    }
    if (!fs.existsSync(customRuleFile) || !fs.statSync(customRuleFile).isFile()) {
      vscode.window.showWarningMessage('Custom rule file does not exist or is not a file.');
      return;
    }
    let ruleContent = '';
    try {
      ruleContent = fs.readFileSync(customRuleFile, 'utf-8');
    } catch (err) {
      vscode.window.showErrorMessage('Failed to read custom rule file.');
      return;
    }
    // Tạo prompt KHÔNG include codeContent, chỉ hướng dẫn Copilot kiểm tra file đang mở
    const prompt = `Please check the currently open C++ file for violations of the following custom rules (markdown format):\n\n${ruleContent}\n\nReport all violations in detail, including line numbers, rule references, and suggested fixes.`;
    await vscode.env.clipboard.writeText(prompt);
    vscode.window.showInformationMessage('Custom rule prompt has been copied to clipboard.');
  });
  context.subscriptions.push(disposable);
}
