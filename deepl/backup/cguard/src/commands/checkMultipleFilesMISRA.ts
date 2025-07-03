import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import { loadRules, connectToCopilot, selectBestModel, createFocusedPrompt, runAnalysisWithProgress, saveReport } from '../helpers/copilot';
import { getReportDirectory } from '../extension';

export function registerCheckMultipleFilesMISRACommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.checkMultipleFilesMISRA', async () => {
    const folderUri = await vscode.window.showOpenDialog({
      canSelectFolders: true,
      canSelectFiles: false,
      canSelectMany: false,
      openLabel: 'Select Source Folder to Scan'
    });
    if (!folderUri || folderUri.length === 0) {
      vscode.window.showInformationMessage('No folder selected.');
      return;
    }
    const srcPath = folderUri[0].fsPath;
    async function findCppFiles(dir: string): Promise<string[]> {
      let results: string[] = [];
      const entries = await fs.promises.readdir(dir, { withFileTypes: true });
      for (const entry of entries) {
        const fullPath = path.join(dir, entry.name);
        if (entry.isDirectory()) {
          results = results.concat(await findCppFiles(fullPath));
        } else {
          const ext = path.extname(fullPath).toLowerCase();
          if (ext === '.cpp' || ext === '.cc') results.push(fullPath);
        }
      }
      return results;
    }
    const cppFiles = await findCppFiles(srcPath);
    if (cppFiles.length === 0) {
      vscode.window.showInformationMessage('No C++ files found in selected folder.');
      return;
    }
    const outputChannel = vscode.window.createOutputChannel('CodeGuard - MISRA Multi-file');
    outputChannel.clear();
    outputChannel.show(true);
    outputChannel.appendLine(`🔎 Found ${cppFiles.length} C++ files. Starting MISRA analysis...`);
    const rulesContent = await loadRules('MISRA', outputChannel);
    if (!rulesContent || rulesContent.trim().length === 0) {
      outputChannel.appendLine('❌ No MISRA rules loaded - analysis cannot proceed');
      vscode.window.showErrorMessage('Failed to load MISRA rule files');
      return;
    }
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
    const defectReportDir = getReportDirectory();
    let successCount = 0;
    for (let i = 0; i < cppFiles.length; ++i) {
      const filePath = cppFiles[i];
      try {
        const codeContent = await fs.promises.readFile(filePath, 'utf8');
        const prompt = createFocusedPrompt(codeContent, rulesContent, 'MISRA');
        outputChannel.appendLine(`\n[${i+1}/${cppFiles.length}] Analyzing: ${filePath}`);
        const timestamp = new Date().toISOString();
        const response = await runAnalysisWithProgress(selectedModel, prompt, outputChannel);
        await saveReport(filePath, 'MISRA', response, codeContent, selectedModel.name, timestamp, outputChannel, defectReportDir);
        outputChannel.appendLine(`✅ Done: ${filePath}`);
        ++successCount;
      } catch (err) {
        const errMsg = err instanceof Error ? err.message : String(err);
        outputChannel.appendLine(`❌ Error analyzing ${filePath}: ${errMsg}`);
      }
    }
    outputChannel.appendLine(`\n🎉 MISRA multi-file analysis complete. ${successCount}/${cppFiles.length} files analyzed.`);
    vscode.window.showInformationMessage(`MISRA multi-file analysis complete. ${successCount}/${cppFiles.length} files analyzed.`);
  });
  context.subscriptions.push(disposable);
}
