import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import { generateInteractiveHTMLReport } from '../helpers/report';
import { isCppFile } from '../helpers/fileUtils';

export function registerGenerateHTMLReportCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.generateHTMLReport', async () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
      vscode.window.showErrorMessage('No active editor found');
      return;
    }
    const filePath = editor.document.uri.fsPath;
    if (!isCppFile(filePath)) {
      vscode.window.showErrorMessage('This command only works with C++ files');
      return;
    }
    const ruleChoice = await vscode.window.showQuickPick([
      { label: 'LGEDV', description: 'Generate HTML report for LGEDV analysis' },
      { label: 'MISRA', description: 'Generate HTML report for MISRA analysis' },
      { label: 'CERT', description: 'Generate HTML report for CERT analysis' },
      { label: 'RAPIDSCAN', description: 'Generate HTML report for RapidScan analysis' },
      { label: 'CRITICAL', description: 'Generate HTML report for Critical analysis' }
    ], {
      placeHolder: 'Select rule type for HTML report generation',
      title: 'Generate Interactive HTML Report'
    });
    if (!ruleChoice) return;
    const ruleType = ruleChoice.label;
    const baseFileName = path.basename(filePath, path.extname(filePath));
    const defectReportDir = '/home/worker/src/defect_report';
    const mdReportPath = path.join(defectReportDir, `${baseFileName}-${ruleType.toLowerCase()}-report.md`);
    try {
      if (!fs.existsSync(mdReportPath)) {
        vscode.window.showErrorMessage('Markdown report not found. Please run analysis first.');
        return;
      }
      const mdContent = await fs.promises.readFile(mdReportPath, 'utf8');
      const analysisMatch = mdContent.match(/## 📋 Analysis Results\s*\n\n([\s\S]*?)\n\n---/);
      const sourceMatch = mdContent.match(/## 📄 Source Code Analyzed\s*\n\n```cpp\n([\s\S]*?)\n```/);
      if (!analysisMatch || !sourceMatch) {
        vscode.window.showErrorMessage('Failed to extract analysis or source code from markdown report.');
        return;
      }
      const analysisResults = analysisMatch[1];
      const sourceCode = sourceMatch[1];
      const timestamp = new Date().toISOString();
      const htmlReport = await generateInteractiveHTMLReport(
        baseFileName, filePath, ruleType, analysisResults, sourceCode, 'GitHub Copilot', timestamp
      );
      if (!fs.existsSync(defectReportDir)) {
        fs.mkdirSync(defectReportDir, { recursive: true });
      }
      const htmlPath = path.join(defectReportDir, `${baseFileName}-${ruleType.toLowerCase()}-report.html`);
      await fs.promises.writeFile(htmlPath, htmlReport, 'utf8');
      vscode.window.showInformationMessage(`✅ HTML report generated: ${path.basename(htmlPath)}`);
    } catch (error) {
      vscode.window.showErrorMessage(`Error generating HTML report: ${error}`);
    }
  });
  context.subscriptions.push(disposable);
}
