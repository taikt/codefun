// Các hàm connect/select model Copilot
import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import { generateMarkdownReport, generateInteractiveHTMLReport } from './report';
import { fileExists } from './fileUtils';

export async function connectToCopilot(outputChannel: vscode.OutputChannel): Promise<vscode.LanguageModelChat[]> {
  const priorityFamilies = [
    { vendor: 'copilot', family: 'gpt-4o' },
    { vendor: 'copilot', family: 'gpt-4o-mini' },
    { vendor: 'copilot', family: 'o1' },
    { vendor: 'copilot', family: 'o1-mini' },
    { vendor: 'copilot', family: 'claude-3.5-sonnet' },
    { vendor: 'copilot' } // fallback: all copilot models
  ];
  for (const query of priorityFamilies) {
    try {
      outputChannel.appendLine(`🔄 Connecting to Copilot model: ${query.family || 'any'}...`);
      const models = await Promise.race([
        vscode.lm.selectChatModels({ vendor: query.vendor, family: query.family }),
        new Promise<vscode.LanguageModelChat[]>((_, reject) => setTimeout(() => reject(new Error('Connection timeout')), 5000))
      ]);
      if (models.length > 0) {
        outputChannel.appendLine(`✅ Found model(s): ${models.map(m => m.name).join(', ')}`);
        return models;
      }
    } catch (err) {
      outputChannel.appendLine(`⚠️ Model connect failed: ${err}`);
    }
  }
  return [];
}

export function selectBestModel(models: vscode.LanguageModelChat[]): vscode.LanguageModelChat {
  const priorities = [
    (m: any) => m.name.toLowerCase().includes('4.1') || m.name.toLowerCase().includes('4-turbo'),
    (m: any) => m.name.toLowerCase() === 'gpt-4o',
    (m: any) => m.name.toLowerCase().includes('gpt-4o-mini'),
    (m: any) => m.name.toLowerCase() === 'o1',
    (m: any) => m.name.toLowerCase() === 'o1-mini',
    (m: any) => m.name.toLowerCase().includes('claude-3.5-sonnet'),
    (m: any) => m.name.toLowerCase().includes('gpt-4') && !m.name.toLowerCase().includes('3.5'),
    (m: any) => m.name.toLowerCase().includes('3.5'),
  ];
  for (const match of priorities) {
    const found = models.find(match);
    if (found) return found;
  }
  return models[0];
}

export async function runRuleAnalysis(ruleType: 'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL') {
  const editor = vscode.window.activeTextEditor;
  if (!editor) {
    vscode.window.showErrorMessage('No active editor found. Please open a C++ file.');
    return;
  }
  if (editor.document.languageId !== 'cpp') {
    vscode.window.showErrorMessage('This extension only works with C++ files (.cpp, .h, .hpp)');
    return;
  }
  const channelName = `CodeGuard - ${ruleType} Analysis`;
  const outputChannel = vscode.window.createOutputChannel(channelName);
  outputChannel.clear();
  outputChannel.show(true);
  const filePath = editor.document.uri.fsPath;
  const codeContent = editor.document.getText();
  // uncomment log code content for Copilot debugging
  // await logCodeContentForCopilot(filePath, codeContent);
  const timestamp = new Date().toISOString();
  outputChannel.appendLine(`🛡️ LGEDV CodeGuard - ${ruleType} Analysis`);
  outputChannel.appendLine('='.repeat(60));
  outputChannel.appendLine(`📁 File: ${path.basename(filePath)}`);
  outputChannel.appendLine(`📍 Path: ${filePath}`);
  outputChannel.appendLine(`⏰ Started: ${new Date().toLocaleString()}`);
  outputChannel.appendLine(`📊 Code size: ${codeContent.length} characters`);
  outputChannel.appendLine(`🎯 Analysis Type: ${ruleType} rules`);
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
    outputChannel.appendLine(`📖 Loading ${ruleType} rules...`);
    const rulesContent = await loadRules(ruleType, outputChannel);
    if (!rulesContent || rulesContent.trim().length === 0) {
      outputChannel.appendLine('❌ No rules loaded');
      vscode.window.showErrorMessage('Failed to load rule files');
      return;
    }
    const prompt = createFocusedPrompt(codeContent, rulesContent, ruleType);
    outputChannel.appendLine(`🔍 Analyzing against ${ruleType} rules...`);
    outputChannel.appendLine('');
    const response = await runAnalysisWithProgress(selectedModel, prompt, outputChannel);
    outputChannel.appendLine('');
    outputChannel.appendLine(`📋 ${ruleType} ANALYSIS RESULTS:`);
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
    await saveReport(filePath, ruleType, response, codeContent, selectedModel.name, timestamp, outputChannel, defectReportDir);
    vscode.window.showInformationMessage(`✅ ${ruleType} analysis complete! HTML report generated in configured directory.`);
  } catch (error) {
    const errorMsg = error instanceof Error ? error.message : 'Unknown error';
    outputChannel.appendLine(`\n❌ ANALYSIS FAILED: ${errorMsg}`);
    outputChannel.appendLine('\n🔧 Troubleshooting:');
    outputChannel.appendLine('• Check GitHub Copilot subscription status');
    outputChannel.appendLine('• Ensure stable internet connection');
    outputChannel.appendLine('• Try with smaller code files (<10KB)');
    outputChannel.appendLine('• Restart VS Code if problems persist');
    vscode.window.showErrorMessage(`${ruleType} analysis failed: ${errorMsg}`);
  }
}

export async function openCopilotChatWithPrompt(prompt: string, ruleType: string) {
  const copilotChatCmds = [
    'github.copilotChat.open',
    'github.copilot-chat.open'
  ];
  let found = false;
  for (const cmd of copilotChatCmds) {
    const all = await vscode.commands.getCommands(true);
    if (all.includes(cmd)) {
      await vscode.commands.executeCommand(cmd);
      await vscode.env.clipboard.writeText(prompt);
      vscode.window.showInformationMessage(
        `${ruleType} prompt has been copied to clipboard. Please paste it into Copilot Chat.`,
        'Open Copilot Chat'
      );
      found = true;
      break;
    }
  }
  if (!found) {
    await vscode.env.clipboard.writeText(prompt);
    vscode.window.showInformationMessage(
      `${ruleType} prompt has been copied to clipboard. Please open Copilot Chat and paste the prompt to check the rules.`
    );
  }
}

export async function loadRules(ruleType: 'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL', outputChannel: vscode.OutputChannel): Promise<string> {
  // This implementation assumes rule files are in a known directory, update as needed
  const ruleSetDir = path.join(__dirname, '..', 'rule_set');
  let rulesContent = '';
  outputChannel.appendLine(`📂 Rule set directory: ${ruleSetDir}`);
  try {
    if (ruleType === 'LGEDV') {
      const lgedvPath = path.join(ruleSetDir, 'LGEDVRuleGuide.md');
      if (await fileExists(lgedvPath)) {
        rulesContent = await fs.promises.readFile(lgedvPath, 'utf8');
        outputChannel.appendLine('✅ Loaded LGEDV rules');
      } else {
        outputChannel.appendLine('❌ LGEDVRuleGuide.md not found');
      }
    } else if (ruleType === 'MISRA') {
      const misraPath = path.join(ruleSetDir, 'Misracpp2008Guidelines_en.md');
      if (await fileExists(misraPath)) {
        rulesContent = await fs.promises.readFile(misraPath, 'utf8');
        outputChannel.appendLine('✅ Loaded MISRA rules');
      } else {
        outputChannel.appendLine('❌ Misracpp2008Guidelines_en.md not found');
      }
    } else if (ruleType === 'CERT') {
      const certPath = path.join(ruleSetDir, 'CertcppGuidelines_en.md');
      if (await fileExists(certPath)) {
        rulesContent = await fs.promises.readFile(certPath, 'utf8');
        outputChannel.appendLine('✅ Loaded CERT rules');
      } else {
        outputChannel.appendLine('❌ CertcppGuidelines_en.md not found');
      }
    } else if (ruleType === 'RAPIDSCAN') {
      const rapidPath = path.join(ruleSetDir, 'RapidScanGuidelines_en.md');
      if (await fileExists(rapidPath)) {
        rulesContent = await fs.promises.readFile(rapidPath, 'utf8');
        outputChannel.appendLine('✅ Loaded RapidScan rules');
      } else {
        outputChannel.appendLine('❌ RapidScanGuidelines_en.md not found');
      }
    } else if (ruleType === 'CRITICAL') {
      const criticalPath = path.join(ruleSetDir, 'CriticalRuleGuideLines.md');
      if (await fileExists(criticalPath)) {
        rulesContent = await fs.promises.readFile(criticalPath, 'utf8');
        outputChannel.appendLine('✅ Loaded Critical rules');
      } else {
        outputChannel.appendLine('❌ CriticalRuleGuideLines.md not found');
      }
    }
    if (rulesContent.length === 0) {
      outputChannel.appendLine('❌ No rules loaded');
      return '';
    }
    outputChannel.appendLine(`📊 Rules loaded: ${Math.round(rulesContent.length / 1000)}K characters`);
    return rulesContent;
  } catch (error) {
    outputChannel.appendLine(`❌ Error loading rules: ${error}`);
    return '';
  }
}

export function createFocusedPrompt(
  codeContent: string,
  rulesContent: string,
  ruleType: 'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL' | 'CUSTOM'
): string {
  const ruleTypeDescription: Record<'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL' | 'CUSTOM', string> = {
    'LGEDV': 'LGEDV_CRCL rules for automotive code compliance',
    'MISRA': 'MISRA C++ 2008 rules for safety-critical software',
    'CERT': 'CERT C++ Secure Coding Standard rules',
    'RAPIDSCAN': 'RapidScan static analysis rules',
    'CRITICAL': 'Critical LGEDV rules for essential code compliance',
    'CUSTOM': 'the following custom rules (markdown format)'
  };
  return `You are a C++ static analysis expert. Analyze this code for violations of ${ruleTypeDescription[ruleType]}.

**IMPORTANT:**
- Report all line numbers and function ranges according to the original file as provided below, including all comments, includes, and blank lines. Do NOT renumber or skip any lines. Your line numbers must match exactly with the code block below.

**CODE TO ANALYZE:**
\`\`\`cpp
${codeContent}
\`\`\`

**RULES TO CHECK:**
${rulesContent}

**ANALYSIS REQUIREMENTS:**
- Find ALL violations of the rules above
- Focus specifically on ${ruleType === 'CRITICAL' ? 'CRITICAL' : ruleType} rule violations
- Cite EXACT rule numbers (e.g., LGEDV_CRCL_0001, Rule 8-4-3, DCL50-CPP, RS-001)
- Check every line thoroughly
- Provide concrete fixes for each violation
- Use the original file's line numbers in all reports

**OUTPUT FORMAT:**
For each violation found:

## 🚨 Issue [#]: [Brief Description]
**Rule Violated:** [EXACT_RULE_NUMBER] - [Rule Description]
**Line:** [LINE_NUMBER]
**Severity:** [Critical/High/Medium/Low]
**Current Code:**
\`\`\`cpp
[problematic code]
\`\`\`
**Fixed Code:**
\`\`\`cpp
[corrected code]
\`\`\`
**Explanation:** [Why this violates the rule and how fix works]

---

## 🔧 Complete Fixed Code
\`\`\`cpp
[entire corrected file with all fixes applied]
\`\`\`

**Important:** If no violations are found, clearly state "No ${ruleType} rule violations detected in this code."`;
}

export async function runAnalysisWithProgress(
  model: vscode.LanguageModelChat,
  prompt: string,
  outputChannel: vscode.OutputChannel
): Promise<string> {
  const messages = [vscode.LanguageModelChatMessage.User(prompt)];
  const cancellationToken = new vscode.CancellationTokenSource();
  const timeoutId = setTimeout(() => {
    cancellationToken.cancel();
    outputChannel.appendLine('⚠️ Analysis timeout after 30 seconds');
  }, 30000);
  const request = await model.sendRequest(messages, {}, cancellationToken.token);
  let response = '';
  let charCount = 0;
  let lastUpdate = Date.now();
  for await (const fragment of request.text) {
    response += fragment;
    charCount += fragment.length;
    const now = Date.now();
    if ((now - lastUpdate) > 3000) {
      const progress = Math.min(Math.floor((charCount / 2000) * 100), 99);
      outputChannel.appendLine(`⏳ Analyzing... ${progress}% (${Math.round(charCount/1000)}K chars)`);
      lastUpdate = now;
    }
  }
  clearTimeout(timeoutId);
  return response;
}

export async function saveReport(
  filePath: string,
  ruleType: string,
  response: string,
  codeContent: string,
  modelName: string,
  timestamp: string,
  outputChannel: vscode.OutputChannel,
  defectReportDir?: string // thêm tham số tuỳ chọn
) {
  const baseFileName = path.basename(filePath, path.extname(filePath));
  let reportDir = defectReportDir;
  if (!reportDir) {
    // Nếu không truyền vào, mặc định tạo defect_report trong workspace folder
    const wsFolder = vscode.workspace.workspaceFolders?.[0]?.uri.fsPath;
    if (wsFolder) {
      reportDir = path.join(wsFolder, 'defect_report');
    } else {
      reportDir = path.resolve('defect_report');
    }
  }
  // Đảm bảo reportDir luôn là string
  const safeReportDir: string = reportDir!;
  if (!fs.existsSync(safeReportDir)) {
    fs.mkdirSync(safeReportDir, { recursive: true });
    outputChannel.appendLine(`📁 Created directory: ${safeReportDir}`);
  } else {
    outputChannel.appendLine(`📁 Using directory: ${safeReportDir}`);
  }
  outputChannel.appendLine('📝 Generating dual reports (Markdown + HTML)...');
  try {
    const mdReport = await generateMarkdownReport(baseFileName, filePath, ruleType, response, codeContent, modelName, timestamp);
    const mdPath = path.join(safeReportDir, `${baseFileName}-${ruleType.toLowerCase()}-report.md`);
    await fs.promises.writeFile(mdPath, mdReport, 'utf8');
    outputChannel.appendLine(`✅ Markdown report: ${path.basename(mdPath)} → ${safeReportDir}`);
    const htmlReport = await generateInteractiveHTMLReport(baseFileName, filePath, ruleType, response, codeContent, modelName, timestamp);
    const htmlPath = path.join(safeReportDir, `${baseFileName}-${ruleType.toLowerCase()}-report.html`);
    await fs.promises.writeFile(htmlPath, htmlReport, 'utf8');
    outputChannel.appendLine(`✅ HTML report: ${path.basename(htmlPath)} → ${safeReportDir}`);
    outputChannel.appendLine(`📂 All reports saved to: ${safeReportDir}`);
    return { mdPath, htmlPath };
  } catch (error) {
    outputChannel.appendLine(`❌ Error saving reports: ${error}`);
    throw error;
  }
}

export function logCodeContentForCopilot(filePath: string, code: string): void {
    const fs = require('fs');
    const logPath = `${filePath}.copilot_sent.log`;
    fs.writeFileSync(logPath, code, 'utf8');
}

export function optimizeRulesForPrompt(rules: string, maxLength: number): string {
    if (rules.length <= maxLength) return rules;
    // Simple truncation for now; can be improved
    return rules.slice(0, maxLength);
}
