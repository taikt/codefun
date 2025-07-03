import * as vscode from 'vscode';
import { openCopilotChatWithPrompt } from '../helpers/copilot';
import { isCppFile } from '../helpers/fileUtils';

export function registerGenerateCertCppPromptCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.generateCertCppPrompt', async () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
      vscode.window.showErrorMessage('No active editor');
      return;
    }
    const document = editor.document;
    const filePath = document.uri.fsPath;
    if (!isCppFile(filePath)) {
      vscode.window.showErrorMessage('Selected file is not a C++ file');
      return;
    }
    const prompt = `Check for C++ rule violation based on the guidelines in CertcppGuidelines_en.md. If the guidelines file is not existed, please download the file by calling fetch_certcpp_rule from MCP server.\nExplain rule content if a rule is violated and show code line number of code that violoate the rule.\nSuggest refactored code to comply with these rules.`;
    await openCopilotChatWithPrompt(prompt, 'CERT C++');
  });
  context.subscriptions.push(disposable);
}
