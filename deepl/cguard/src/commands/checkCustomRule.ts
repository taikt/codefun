import * as vscode from 'vscode';
import * as fs from 'fs';
import * as path from 'path';
import { connectToCopilot, selectBestModel, runAnalysisWithProgress, saveReport, createFocusedPrompt } from '../helpers/copilot';

export function registerCheckCustomRuleCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.checkCustomRule', async () => {
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
        'Custom rule file path is empty. Please set the path to your markdown rule file in settings (lgedvCodeGuard.customRuleFile).'
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
    let codeContent = '';
    try {
      codeContent = document.getText();
    } catch (err) {
      vscode.window.showErrorMessage('Failed to read C++ file content.');
      return;
    }
    // Tạo output channel
    const channelName = `CodeGuard - CUSTOM Analysis`;
    const outputChannel = vscode.window.createOutputChannel(channelName);
    outputChannel.clear();
    outputChannel.show(true);
    const timestamp = new Date().toISOString();
    outputChannel.appendLine(`🛡️ LGEDV CodeGuard - CUSTOM Analysis`);
    outputChannel.appendLine('='.repeat(60));
    outputChannel.appendLine(`📁 File: ${path.basename(filePath)}`);
    outputChannel.appendLine(`📍 Path: ${filePath}`);
    outputChannel.appendLine(`⏰ Started: ${new Date().toLocaleString()}`);
    outputChannel.appendLine(`📊 Code size: ${codeContent.length} characters`);
    outputChannel.appendLine(`🎯 Analysis Type: CUSTOM rules`);
    outputChannel.appendLine('='.repeat(60));
    outputChannel.appendLine('');
    try {
      outputChannel.appendLine('🔍 Connecting to GitHub Copilot...');
      const models = await connectToCopilot(outputChannel);
      if (models.length === 0) {
        outputChannel.appendLine('❌ No Copilot models available');
        vscode.window.showErrorMessage('GitHub Copilot not available. Please check your subscription.');
        return;
      }
      const selectedModel = selectBestModel(models);
      outputChannel.appendLine(`✅ Connected: ${selectedModel.name}`);
      outputChannel.appendLine(`✅ Connected model: ${JSON.stringify(selectedModel, null, 2)}`);
      outputChannel.appendLine('');
      outputChannel.appendLine(`📖 Loading CUSTOM rules...`);
      outputChannel.appendLine('✅ Loaded custom rules from file');
      const prompt = createFocusedPrompt(codeContent, ruleContent, 'CUSTOM');
      outputChannel.appendLine(`🔍 Analyzing against CUSTOM rules...`);
      outputChannel.appendLine('');
      const response = await runAnalysisWithProgress(selectedModel, prompt, outputChannel);
      outputChannel.appendLine('');
      outputChannel.appendLine(`📋 CUSTOM ANALYSIS RESULTS:`);
      outputChannel.appendLine('='.repeat(55));
      outputChannel.appendLine(response);
      outputChannel.appendLine('');
      outputChannel.appendLine('='.repeat(55));
      outputChannel.appendLine('✅ ANALYSIS COMPLETE!');
      outputChannel.appendLine(`⏱️ Total time: ${Math.round((Date.now() - Date.parse(timestamp)) / 1000)}s`);
      // Lấy defectReportDir từ workspace (nếu có)
      let defectReportDir: string | undefined = undefined;
      const wsFolder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;
      if (wsFolder) {
        defectReportDir = path.join(wsFolder, 'defect_report');
      }
      await saveReport(filePath, 'CUSTOM', response, codeContent, selectedModel.name, timestamp, outputChannel, defectReportDir);
      vscode.window.showInformationMessage(`✅ CUSTOM analysis complete! Markdown and HTML report generated in configured directory.`);
    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : 'Unknown error';
      outputChannel.appendLine(`\n❌ ANALYSIS FAILED: ${errorMsg}`);
      outputChannel.appendLine('\n🔧 Troubleshooting:');
      outputChannel.appendLine('• Check GitHub Copilot subscription status');
      outputChannel.appendLine('• Ensure stable internet connection');
      outputChannel.appendLine('• Try with smaller code files (<10KB)');
      outputChannel.appendLine('• Restart VS Code if problems persist');
      vscode.window.showErrorMessage(`CUSTOM analysis failed: ${errorMsg}`);
    }
  });
  context.subscriptions.push(disposable);
}
