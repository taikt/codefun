import * as vscode from 'vscode';
import { exec, spawn } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';

// Helper functions from extension_refer.ts
// Function to strip ANSI escape codes
const stripAnsi = (text: string): string => {
  return text.replace(/\x1B\[[0-?]*[ -/]*[@-~]/g, '');
};

// Check if a file is a C++ file based on its extension
const isCppFile = (filename: string): boolean => {
  const cppExtensions = ['.cpp', '.h', '.hpp', '.cxx', '.cc'];
  return cppExtensions.includes(path.extname(filename).toLowerCase());
};

export function activate(context: vscode.ExtensionContext) {
  
  // COMMAND 1: Check LGEDV Rules Only
  let checkLGEDVCommand = vscode.commands.registerCommand('lgedv.checkLGEDVRules', async () => {
    await runRuleAnalysis('LGEDV');
  });

  // COMMAND 2: Check MISRA-High Rules Only  
  let checkMISRAHighCommand = vscode.commands.registerCommand('lgedv.checkMISRAHighRules', async () => {
    await runRuleAnalysis('MISRA-HIGH');
  });

  // COMMAND 3: Check MISRA Rules Only
  let checkMISRACommand = vscode.commands.registerCommand('lgedv.checkMISRARules', async () => {
    await runRuleAnalysis('MISRA');
  });

  // COMMAND 4: Check CERTcpp-High Rules Only
  let checkCERTCommand = vscode.commands.registerCommand('lgedv.checkCERTRules', async () => {
    await runRuleAnalysis('CERT');
  });

  // COMMAND 5: Check RapidScan Rules Only
  let checkRapidScanCommand = vscode.commands.registerCommand('lgedv.checkRapidScanRules', async () => {
    await runRuleAnalysis('RAPIDSCAN');
  });

  // COMMAND 6: Check All Rules (Combined)
  let checkAllRulesCommand = vscode.commands.registerCommand('lgedv.checkAllRules', async () => {
    await runRuleAnalysis('ALL');
  });

  // COMMAND 7: Interactive Rule Selection
  let interactiveCheckCommand = vscode.commands.registerCommand('lgedv.interactiveCheck', async () => {
    const choice = await vscode.window.showQuickPick([
      {
        label: '🎯 LGEDV Rules Only',
        description: 'Check only LGEDV_CRCL rules (fast, focused)',
        detail: 'Best for LGEDV compliance check - faster analysis'
      },
      {
        label: '🛡️ MISRA-High Rules Only', 
        description: 'Check only MISRA C++ 2008 High Priority rules (fast, focused)',
        detail: 'Best for MISRA high priority compliance check - faster analysis'
      },
      {
        label: '� MISRA All Rules',
        description: 'Check all MISRA C++ 2008 rules (comprehensive)',
        detail: 'Complete MISRA analysis - may take longer'
      },
      {
        label: '🔐 CERTcpp-High Rules',
        description: 'Check CERT C++ High Priority security rules',
        detail: 'Security-focused analysis for critical vulnerabilities'
      },
      {
        label: '⚡ RapidScan Rules',
        description: 'Check RapidScan vulnerability rules',
        detail: 'Fast vulnerability scanning for common issues'
      },
      {
        label: '🔍 All Rules Combined',
        description: 'Check LGEDV + MISRA + CERT + RapidScan rules (comprehensive)', 
        detail: 'Complete analysis but may take longer'
      }
    ], {
      placeHolder: 'Select which rules to check against your code',
      title: 'LGEDV CodeGuard - Choose Analysis Type'
    });

    if (choice) {
      if (choice.label.includes('LGEDV')) {
        await runRuleAnalysis('LGEDV');
      } else if (choice.label.includes('MISRA-High')) {
        await runRuleAnalysis('MISRA-HIGH');
      } else if (choice.label.includes('MISRA All')) {
        await runRuleAnalysis('MISRA');
      } else if (choice.label.includes('CERTcpp-High')) {
        await runRuleAnalysis('CERT');
      } else if (choice.label.includes('RapidScan')) {
        await runRuleAnalysis('RAPIDSCAN');
      } else {
        await runRuleAnalysis('ALL');
      }
    }
  });

  // COMMAND 5: Generate Rule Prompt (từ extension_refer.ts)
  let generateRulePromptCommand = vscode.commands.registerCommand('lgedv.generateRulePrompt', async () => {
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
    const misraFile = 'Misracpp2008Guidelines_High_en.md';
    const LGEguideFile = 'LGEDVRuleGuide.md';
    const staticFile = 'StaticGuidelines_High_en.md';
    const prompt = `Check for C++ rule violation based on the guidelines in ${misraFile}, ${LGEguideFile} and ${staticFile}.\n`
      + `Explain rule content if a rule is violated and show code line number of code that violoate the rule.\n`
      + `Suggest refactored code to comply with these rules.`;

    const copilotChatCmds = [
      'github.copilotChat.open',
      'github.copilot-chat.open'
    ];
    let found = false;
    for (const cmd of copilotChatCmds) {
      const all = await vscode.commands.getCommands(true);
      if (all.includes(cmd)) {
        await vscode.commands.executeCommand(cmd, { prompt });
        found = true;
        break;
      }
    }
    if (!found) {
      await vscode.env.clipboard.writeText(prompt);
      vscode.window.showInformationMessage(
        'Prompt has been copied to clipboard. Please open Copilot Chat and paste the prompt to check the rules.'
      );
    }
  });

  // COMMAND 6: Check Rules by Static Analysis (từ extension_refer.ts)
  let checkStaticAnalysisCommand = vscode.commands.registerCommand('lgedv.checkStaticAnalysis', () => {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
      vscode.window.showInformationMessage('No active editor');
      return;
    }
    const document = editor.document;
    const filePath = document.uri.fsPath;
    if (!isCppFile(filePath)) {
      vscode.window.showInformationMessage('Selected file is not a C++ file');
      return;
    }
    const reportFilePath = path.join(path.dirname(filePath), `${path.basename(filePath, path.extname(filePath))}.static-report.txt`);
    fs.writeFileSync(reportFilePath, `C++ Static Vulnerability Report\nSource File: ${filePath}\n\n`, { encoding: 'utf-8' });

    // 1. clang-tidy (LGEDV rules)
    const clangTidyCmd = `/home/worker/src/llvm-build/llvm-project/build/bin/clang-tidy "${filePath}" --checks=-*,custom-* --`;
    exec(clangTidyCmd, (error, stdout, stderr) => {
      const linesToExclude = [
        '99 warnings generated.',
        'Suppressed 98 warnings (98 in non-user code).',
        'Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.'
      ];
      const filteredStderr = stderr
        .split('\n')
        .filter((line: string) => !linesToExclude.includes(line.trim()))
        .join('\n');
      const clangTidyContent = `=== Clang-Tidy (LGEDV Rules) ===\n${stdout}\n${filteredStderr ? `Errors:\n${filteredStderr}` : ''}\n`;
      fs.appendFileSync(reportFilePath, clangTidyContent, { encoding: 'utf-8' });

      // 2. cppcheck
      const cppcheckCmd = `cppcheck --enable=all --suppress=missingIncludeSystem "${filePath}"`;
      exec(cppcheckCmd, { encoding: 'utf-8' }, (cppError, cppStdout, cppStderr) => {
        const cppOutput = cppStdout + cppStderr;
        const cppContent = `\n=== Cppcheck Analysis ===\n${cppOutput.trim() ? cppOutput : 'No issues detected.'}\n`;
        fs.appendFileSync(reportFilePath, cppContent, { encoding: 'utf-8' });

        // 3. clang --analyze
        const clangCmd = `clang --analyze "${filePath}"`;
        exec(clangCmd, { encoding: 'utf-8' }, (clangError, clangStdout, clangStderr) => {
          const clangOutput = clangStdout + clangStderr;
          const clangContent = `\n=== Clang Static Analyzer ===\n${clangOutput.trim() ? clangOutput : 'No issues detected.'}\n`;
          fs.appendFileSync(reportFilePath, clangContent, { encoding: 'utf-8' });

          vscode.window.showInformationMessage(`Static analysis report saved at: ${reportFilePath}`);
        });

        if (cppError) {
          vscode.window.showErrorMessage(`Cppcheck Error: ${cppStderr}`);
        }
      });

      if (error) {
        vscode.window.showErrorMessage(`Clang-Tidy Error: ${stderr}`);
      }
    });
  });

  // Main analysis function
  async function runRuleAnalysis(ruleType: 'LGEDV' | 'MISRA' | 'MISRA-HIGH' | 'CERT' | 'RAPIDSCAN' | 'ALL') {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
      vscode.window.showErrorMessage('No active editor found. Please open a C++ file.');
      return;
    }

    if (editor.document.languageId !== 'cpp') {
      vscode.window.showErrorMessage('This extension only works with C++ files (.cpp, .h, .hpp)');
      return;
    }

    // Create rule-specific output channel
    const channelName = `CodeGuard - ${ruleType} Analysis`;
    const outputChannel = vscode.window.createOutputChannel(channelName);
    outputChannel.clear();
    outputChannel.show(true);

    const filePath = editor.document.uri.fsPath;
    const codeContent = editor.document.getText();
    const timestamp = new Date().toISOString();
    
    // Clear, informative header
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
      // Fast Copilot connection
      outputChannel.appendLine('🔍 Connecting to GitHub Copilot...');
      const models = await connectToCopilot(outputChannel);
      
      if (models.length === 0) {
        outputChannel.appendLine('❌ No Copilot models available');
        outputChannel.appendLine('💡 Please check your GitHub Copilot subscription');
        vscode.window.showErrorMessage('GitHub Copilot not available. Please check your subscription.');
        return;
      }

      const selectedModel = selectBestModel(models);
      outputChannel.appendLine(`✅ Connected: ${selectedModel.name}`);
      outputChannel.appendLine('');

      // Load rules efficiently
      outputChannel.appendLine(`📖 Loading ${ruleType} rules...`);
      const rulesContent = await loadRules(ruleType, outputChannel);
      
      if (!rulesContent || rulesContent.trim().length === 0) {
        outputChannel.appendLine('❌ No rules loaded - analysis cannot proceed');
        vscode.window.showErrorMessage('Failed to load rule files');
        return;
      }

      // Validate rule content patterns (khái quát hóa)
      const { hasLGEDV, hasMISRA } = validateRuleContent(rulesContent, ruleType, outputChannel);
      
      // Create focused prompt based on rule type
      const prompt = createFocusedPrompt(codeContent, rulesContent, ruleType);
      outputChannel.appendLine(`🔍 Analyzing against ${ruleType} rules...`);
      outputChannel.appendLine('');

      // Run analysis with progress tracking
      const response = await runAnalysisWithProgress(selectedModel, prompt, outputChannel);
      
      // Display clean results
      outputChannel.appendLine('');
      outputChannel.appendLine(`📋 ${ruleType} ANALYSIS RESULTS:`);
      outputChannel.appendLine('='.repeat(55));
      outputChannel.appendLine(response);
      outputChannel.appendLine('');
      outputChannel.appendLine('='.repeat(55));
      outputChannel.appendLine('✅ ANALYSIS COMPLETE!');
      outputChannel.appendLine(`⏱️ Total time: ${Math.round((Date.now() - Date.parse(timestamp)) / 1000)}s`);

      // Save detailed report
      await saveReport(filePath, ruleType, response, codeContent, selectedModel.name, timestamp, outputChannel);
      
      // Show completion notification
      const reportFileName = `${path.basename(filePath, path.extname(filePath))}-${ruleType.toLowerCase()}-report.md`;
      const action = await vscode.window.showInformationMessage(
        `✅ ${ruleType} analysis complete! Report saved.`,
        'View Report',
        'Show Output'
      );
      
      if (action === 'View Report') {
        const reportPath = path.join(path.dirname(filePath), reportFileName);
        const doc = await vscode.workspace.openTextDocument(reportPath);
        await vscode.window.showTextDocument(doc);
      } else if (action === 'Show Output') {
        outputChannel.show(true);
      }

    } catch (error) {
      const errorMsg = error instanceof Error ? error.message : 'Unknown error';
      outputChannel.appendLine(`\n❌ ANALYSIS FAILED: ${errorMsg}`);
      
      // Helpful troubleshooting
      outputChannel.appendLine('\n🔧 Troubleshooting:');
      outputChannel.appendLine('• Check GitHub Copilot subscription status');
      outputChannel.appendLine('• Ensure stable internet connection');
      outputChannel.appendLine('• Try with smaller code files (<10KB)');
      outputChannel.appendLine('• Restart VS Code if problems persist');
      
      vscode.window.showErrorMessage(`${ruleType} analysis failed: ${errorMsg}`);
    }
  }

  // Helper: Fast Copilot connection with fallback
  async function connectToCopilot(outputChannel: vscode.OutputChannel): Promise<vscode.LanguageModelChat[]> {
    try {
      // Quick connection attempt with 3-second timeout
      const models = await Promise.race([
        vscode.lm.selectChatModels({ vendor: 'copilot' }),
        new Promise<vscode.LanguageModelChat[]>((_, reject) => 
          setTimeout(() => reject(new Error('Connection timeout')), 3000)
        )
      ]);
      
      if (models.length > 0) {
        outputChannel.appendLine('✅ Quick connection successful');
        return models;
      }
    } catch (timeoutError) {
      outputChannel.appendLine('⚠️ Quick connect failed, trying fallback...');
    }

    // Parallel fallback attempts
    const fallbackPromises = [
      vscode.lm.selectChatModels({ vendor: 'copilot', family: 'gpt-4' }),
      vscode.lm.selectChatModels({ vendor: 'copilot', family: 'gpt-3.5-turbo' })
    ];
    
    const results = await Promise.allSettled(fallbackPromises);
    
    for (const result of results) {
      if (result.status === 'fulfilled' && result.value.length > 0) {
        outputChannel.appendLine('✅ Fallback connection successful');
        return result.value;
      }
    }

    return [];
  }

  // Helper: Select best available model
  function selectBestModel(models: vscode.LanguageModelChat[]): vscode.LanguageModelChat {
    // Prefer GPT-4.1 > GPT-4 > GPT-3.5
    const gpt41Models = models.filter(m => 
      m.name.includes('4.1') || m.name.includes('4o') || m.name.includes('4-turbo')
    );
    if (gpt41Models.length > 0) return gpt41Models[0];

    const gpt4Models = models.filter(m => 
      m.name.toLowerCase().includes('gpt-4') && !m.name.includes('3.5')
    );
    if (gpt4Models.length > 0) return gpt4Models[0];

    return models[0]; // Fallback to first available
  }

  // Helper: Load rules based on type
  async function loadRules(ruleType: 'LGEDV' | 'MISRA' | 'MISRA-HIGH' | 'CERT' | 'RAPIDSCAN' | 'ALL', outputChannel: vscode.OutputChannel): Promise<string> {
    const ruleSetDir = path.join(__dirname, '..', 'src', 'rule_set'); // Fix path to src/rule_set
    const copilotMdDir = '/home/worker/src/copilot_md';
    let rulesContent = '';

    try {
      // LGEDV Rules
      if (ruleType === 'LGEDV' || ruleType === 'ALL') {
        const lgedvContent = await loadRuleFile('LGEDVRuleGuide.md', ruleSetDir, copilotMdDir, outputChannel);
        if (lgedvContent) {
          rulesContent += `# LGEDV Rules\n${lgedvContent}\n\n`;
          outputChannel.appendLine('✅ LGEDV rules loaded');
        } else {
          outputChannel.appendLine('⚠️ LGEDV rules not found');
        }
      }

      // MISRA-High Rules (Misracpp2008Guidelines_High_en.md)
      if (ruleType === 'MISRA-HIGH' || ruleType === 'ALL') {
        const misraHighContent = await loadRuleFile('Misracpp2008Guidelines_High_en.md', ruleSetDir, copilotMdDir, outputChannel);
        if (misraHighContent) {
          rulesContent += `# MISRA C++ 2008 High Priority Rules\n${misraHighContent}\n\n`;
          outputChannel.appendLine('✅ MISRA-High rules loaded');
        } else {
          outputChannel.appendLine('⚠️ MISRA-High rules not found');
        }
      }

      // MISRA All Rules (Misracpp2008Guidelines_en.md)
      if (ruleType === 'MISRA' || ruleType === 'ALL') {
        const misraContent = await loadRuleFile('Misracpp2008Guidelines_en.md', ruleSetDir, copilotMdDir, outputChannel);
        if (misraContent) {
          rulesContent += `# MISRA C++ 2008 All Rules\n${misraContent}\n\n`;
          outputChannel.appendLine('✅ MISRA rules loaded');
        } else {
          outputChannel.appendLine('⚠️ MISRA rules not found');
        }
      }

      // CERT C++ Rules
      if (ruleType === 'CERT' || ruleType === 'ALL') {
        const certContent = await loadRuleFile('CertcppGuidelines_High_en.md', ruleSetDir, copilotMdDir, outputChannel);
        if (certContent) {
          rulesContent += `# CERT C++ High Priority Rules\n${certContent}\n\n`;
          outputChannel.appendLine('✅ CERT rules loaded');
        } else {
          outputChannel.appendLine('⚠️ CERT rules not found');
        }
      }

      // RapidScan Rules
      if (ruleType === 'RAPIDSCAN' || ruleType === 'ALL') {
        const rapidScanContent = await loadRuleFile('RapidScanGuidelines_en.md', ruleSetDir, copilotMdDir, outputChannel);
        if (rapidScanContent) {
          rulesContent += `# RapidScan Vulnerability Rules\n${rapidScanContent}\n\n`;
          outputChannel.appendLine('✅ RapidScan rules loaded');
        } else {
          outputChannel.appendLine('⚠️ RapidScan rules not found');
        }
      }

      // Validate and optimize rule content
      if (rulesContent.length === 0) {
        outputChannel.appendLine('❌ No rules loaded');
        return '';
      }

      outputChannel.appendLine(`📊 Rules loaded: ${Math.round(rulesContent.length / 1000)}K characters`);
      
      // Khái quát hóa: Validate rule patterns thay vì hard-code
      const validation = validateRuleContent(rulesContent, ruleType, outputChannel);
      
      // Optimize prompt size for single rule type vs combined
      const maxSize = ruleType === 'ALL' ? 30000 : 40000; // More space for single rule type
      if (rulesContent.length > maxSize) {
        outputChannel.appendLine(`⚠️ Rules too large (${rulesContent.length}), optimizing to ${maxSize} chars`);
        rulesContent = optimizeRulesForPrompt(rulesContent, ruleType, maxSize);
        outputChannel.appendLine('✅ Rules optimized for best analysis');
        
        // Re-validate after optimization
        validateRuleContent(rulesContent, ruleType, outputChannel);
      }

      return rulesContent;

    } catch (error) {
      outputChannel.appendLine(`❌ Error loading rules: ${error}`);
      return '';
    }
  }

  // Helper: Load individual rule file with fallback
  async function loadRuleFile(fileName: string, primaryDir: string, fallbackDir: string, outputChannel: vscode.OutputChannel): Promise<string | null> {
    const primaryPath = path.join(primaryDir, fileName);
    const fallbackPath = path.join(fallbackDir, fileName);

    outputChannel.appendLine(`🔍 Looking for ${fileName}:`);
    outputChannel.appendLine(`   Primary: ${primaryPath}`);
    outputChannel.appendLine(`   Fallback: ${fallbackPath}`);

    try {
      // Try primary location first
      if (await fileExists(primaryPath)) {
        outputChannel.appendLine(`✅ Found at primary location`);
        return await fs.promises.readFile(primaryPath, 'utf8');
      }
      
      // Try fallback location
      if (await fileExists(fallbackPath)) {
        outputChannel.appendLine(`📂 Using fallback: ${path.basename(fallbackDir)}/${fileName}`);
        return await fs.promises.readFile(fallbackPath, 'utf8');
      }

      outputChannel.appendLine(`❌ File not found in either location`);

    } catch (error) {
      outputChannel.appendLine(`❌ Error reading ${fileName}: ${error}`);
    }

    return null;
  }

  // Helper: Check file existence
  async function fileExists(filePath: string): Promise<boolean> {
    try {
      await fs.promises.access(filePath);
      return true;
    } catch {
      return false;
    }
  }

  // Helper: Optimize rules for prompt efficiency
  function optimizeRulesForPrompt(rulesContent: string, ruleType: string, maxSize: number): string {
    if (ruleType === 'ALL') {
      // For combined analysis, balance LGEDV and MISRA
      const lgedvMatch = rulesContent.match(/# LGEDV Rules\n([\s\S]*?)(?=\n# |$)/);
      const misraMatch = rulesContent.match(/# MISRA C\+\+ 2008 Rules\n([\s\S]*?)(?=\n# |$)/);
      
      if (lgedvMatch && misraMatch) {
        const lgedvSpace = Math.floor(maxSize * 0.5);
        const misraSpace = Math.floor(maxSize * 0.5);
        
        const lgedvContent = lgedvMatch[1].substring(0, lgedvSpace);
        const misraContent = misraMatch[1].substring(0, misraSpace);
        
        return `# LGEDV Rules\n${lgedvContent}\n\n# MISRA C++ 2008 Rules\n${misraContent}\n\n`;
      }
    }
    
    // For single rule type or fallback, simple truncation
    return rulesContent.substring(0, maxSize);
  }

  // Helper: Create focused prompt based on rule type
  function createFocusedPrompt(codeContent: string, rulesContent: string, ruleType: 'LGEDV' | 'MISRA' | 'MISRA-HIGH' | 'CERT' | 'RAPIDSCAN' | 'ALL'): string {
    const ruleTypeDescription: Record<'LGEDV' | 'MISRA' | 'MISRA-HIGH' | 'CERT' | 'RAPIDSCAN' | 'ALL', string> = {
      'LGEDV': 'LGEDV_CRCL rules for automotive code compliance',
      'MISRA': 'MISRA C++ 2008 all rules for safety-critical software',
      'MISRA-HIGH': 'MISRA C++ 2008 high priority rules for safety-critical software',
      'CERT': 'CERT C++ high priority security rules for secure coding',
      'RAPIDSCAN': 'RapidScan vulnerability rules for security scanning',
      'ALL': 'LGEDV_CRCL, MISRA C++ 2008, CERT C++, and RapidScan rules'
    };

    return `You are a C++ static analysis expert. Analyze this code for violations of ${ruleTypeDescription[ruleType]}.

**CODE TO ANALYZE:**
\`\`\`cpp
${codeContent}
\`\`\`

**RULES TO CHECK:**
${rulesContent}

**ANALYSIS REQUIREMENTS:**
- Find ALL violations of the rules above
- Focus specifically on ${ruleType === 'ALL' ? 'ALL RULE TYPES' : ruleType} rule violations
- Cite EXACT rule numbers (e.g., LGEDV_CRCL_0001, Rule 8-4-3, CTR50-CPP, sprintf)
- Check every line thoroughly
- Provide concrete fixes for each violation

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

  // Helper: Run analysis with progress tracking
  async function runAnalysisWithProgress(
    model: vscode.LanguageModelChat, 
    prompt: string, 
    outputChannel: vscode.OutputChannel
  ): Promise<string> {
    const messages = [vscode.LanguageModelChatMessage.User(prompt)];
    const cancellationToken = new vscode.CancellationTokenSource();
    
    // Set 30-second timeout
    const timeoutId = setTimeout(() => {
      cancellationToken.cancel();
      outputChannel.appendLine('⚠️ Analysis timeout after 30 seconds');
    }, 30000);
    
    const request = await model.sendRequest(messages, {}, cancellationToken.token);
    
    let response = '';
    let charCount = 0;
    let lastUpdate = Date.now();
    
    // Clean progress updates every 3 seconds
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

  // Helper: Save detailed markdown report
  async function saveReport(
    filePath: string, 
    ruleType: string, 
    response: string, 
    codeContent: string, 
    modelName: string, 
    timestamp: string, 
    outputChannel: vscode.OutputChannel
  ) {
    const reportFileName = `${path.basename(filePath, path.extname(filePath))}-${ruleType.toLowerCase()}-report.md`;
    const reportPath = path.join(path.dirname(filePath), reportFileName);
    
    const markdownReport = `# 🛡️ ${ruleType} Analysis Report

**File:** \`${path.basename(filePath)}\`  
**Path:** \`${filePath}\`  
**Analyzed:** ${new Date(timestamp).toLocaleString()}  
**Rule Type:** ${ruleType}  
**Model:** ${modelName}  
**Code Size:** ${codeContent.length} characters

---

## 📋 Analysis Results

${response}

---

## 📄 Source Code Analyzed

\`\`\`cpp
${codeContent}
\`\`\`

---

*Generated by LGEDV CodeGuard v1.0.0 - ${ruleType} Analysis*
`;

    try {
      await fs.promises.writeFile(reportPath, markdownReport, 'utf8');
      outputChannel.appendLine(`📄 Report saved: ${reportFileName}`);
    } catch (error) {
      outputChannel.appendLine(`⚠️ Could not save report: ${error}`);
    }
  }

  // Helper: Validate rule content patterns (khái quát hóa)
  function validateRuleContent(rulesContent: string, ruleType: 'LGEDV' | 'MISRA' | 'MISRA-HIGH' | 'CERT' | 'RAPIDSCAN' | 'ALL', outputChannel: vscode.OutputChannel): { hasLGEDV: boolean, hasMISRA: boolean } {
    // Detect LGEDV rules by pattern
    const lgedvPatterns = [
      /LGEDV_CRCL_\d+/,           // LGEDV_CRCL_0001, LGEDV_CRCL_0013, etc.
      /Rule LGEDV_CRCL_/,         // Alternative format
      /## Rule LGEDV_CRCL_/       // Markdown header format
    ];
    
    // Detect MISRA rules by pattern
    const misraPatterns = [
      /Rule \d+-\d+-\d+/,         // Rule 6-4-2, Rule 8-5-1, etc.
      /### Rule \d+-\d+-\d+/,     // Markdown header format
      /MISRA.*Rule.*\d+-\d+-\d+/, // Various MISRA rule formats
      /MISRA-C\+\+.*\d+-\d+-\d+/, // MISRA-C++ specific
      /\*\*Rule \d+-\d+-\d+\*\*/  // Bold rule format
    ];

    // Detect CERT rules by pattern
    const certPatterns = [
      /CTR\d+-CPP/,               // CTR50-CPP, CTR51-CPP, etc.
      /DCL\d+-CPP/,               // DCL50-CPP, DCL58-CPP, etc.
      /ERR\d+-CPP/,               // ERR56-CPP, ERR57-CPP, etc.
      /EXP\d+-CPP/,               // EXP50-CPP, EXP52-CPP, etc.
      /MEM\d+-CPP/,               // MEM50-CPP, MEM51-CPP, etc.
      /MSC\d+-CPP/,               // MSC54-CPP, etc.
      /OOP\d+-CPP/,               // OOP55-CPP, OOP57-CPP, etc.
      /STR\d+-CPP/                // STR50-CPP, STR51-CPP, etc.
    ];

    // Detect RapidScan rules by pattern
    const rapidScanPatterns = [
      /sprintf/,                  // sprintf vulnerability
      /sscanf/,                   // sscanf vulnerability
      /strcat/,                   // strcat vulnerability
      /strcpy/,                   // strcpy vulnerability
      /M-C-UNINIT_MEMORY/,        // RapidScan rule patterns
      /M-C-FLOAT_EQUAL/,
      /M-C-MISSING_RETURN/,
      /M-CPP-DEALLOC_PAIR/,
      /R-CPP-NEW_DELETE_PAIR/,
      /R-C-SEQ_POINT/,
      /R-C-ARRAY_OVERRUN/,
      /R-CPP-VIRTUAL_DTOR/,
      /VULDOC_BOF_/,              // Buffer overflow patterns
      /VULDOC_INSUFFICIENT_SPACE_/,
      /VULDOC_VALID_SIZE_/,
      /VULDOC_TOCTOU_/,           // TOCTOU patterns
      /VULDOC_RANDOM_/,           // Random patterns
      /VULDOC_INVALID_ARRAY_INDEX/,
      /VULDOC_UNINITIALIZED_MEMORY/,
      /VULDOC_SYSCALL_/,          // System call patterns
      /VULDOC_INSECURE_/,         // Security patterns
      /VULDOC_SOCKET_WILDCARD_BINDING/
    ];
    
    // Check LGEDV patterns
    const hasLGEDV = lgedvPatterns.some(pattern => pattern.test(rulesContent));
    
    // Check MISRA patterns
    const hasMISRA = misraPatterns.some(pattern => pattern.test(rulesContent));

    // Check CERT patterns
    const hasCERT = certPatterns.some(pattern => pattern.test(rulesContent));

    // Check RapidScan patterns
    const hasRapidScan = rapidScanPatterns.some(pattern => pattern.test(rulesContent));
    
    // Detailed logging
    outputChannel.appendLine(`🔍 Rule validation:`);
    outputChannel.appendLine(`   LGEDV rules detected: ${hasLGEDV ? '✅' : '❌'}`);
    outputChannel.appendLine(`   MISRA rules detected: ${hasMISRA ? '✅' : '❌'}`);
    outputChannel.appendLine(`   CERT rules detected: ${hasCERT ? '✅' : '❌'}`);
    outputChannel.appendLine(`   RapidScan rules detected: ${hasRapidScan ? '✅' : '❌'}`);
    
    // Show rule counts if detected
    if (hasLGEDV) {
      const lgedvCount = (rulesContent.match(/LGEDV_CRCL_\d+/g) || []).length;
      outputChannel.appendLine(`   LGEDV rules found: ${lgedvCount}`);
    }
    
    if (hasMISRA) {
      const misraCount = (rulesContent.match(/Rule \d+-\d+-\d+/g) || []).length;
      outputChannel.appendLine(`   MISRA rules found: ${misraCount}`);
    }

    if (hasCERT) {
      const certCount = (rulesContent.match(/[A-Z]{3}\d+-CPP/g) || []).length;
      outputChannel.appendLine(`   CERT rules found: ${certCount}`);
    }

    if (hasRapidScan) {
      const rapidScanCount = (rulesContent.match(/###\s+\w+/g) || []).length;
      outputChannel.appendLine(`   RapidScan rules found: ${rapidScanCount}`);
    }
    
    // Validation warnings
    if (ruleType === 'LGEDV' && !hasLGEDV) {
      outputChannel.appendLine('⚠️ Warning: LGEDV analysis requested but no LGEDV rules detected');
    }
    
    if ((ruleType === 'MISRA' || ruleType === 'MISRA-HIGH') && !hasMISRA) {
      outputChannel.appendLine('⚠️ Warning: MISRA analysis requested but no MISRA rules detected');
    }

    if (ruleType === 'CERT' && !hasCERT) {
      outputChannel.appendLine('⚠️ Warning: CERT analysis requested but no CERT rules detected');
    }

    if (ruleType === 'RAPIDSCAN' && !hasRapidScan) {
      outputChannel.appendLine('⚠️ Warning: RapidScan analysis requested but no RapidScan rules detected');
    }
    
    if (ruleType === 'ALL' && (!hasLGEDV || !hasMISRA || !hasCERT || !hasRapidScan)) {
      outputChannel.appendLine('⚠️ Warning: Combined analysis requested but some rule types missing');
    }
    
    return { hasLGEDV, hasMISRA };
  }

  // Register all commands
  context.subscriptions.push(
    checkLGEDVCommand, 
    checkMISRAHighCommand,
    checkMISRACommand, 
    checkCERTCommand,
    checkRapidScanCommand,
    checkAllRulesCommand, 
    interactiveCheckCommand,
    generateRulePromptCommand,
    checkStaticAnalysisCommand
  );
  
  // Show activation message
  vscode.window.showInformationMessage('🛡️ LGEDV CodeGuard is ready! Use Ctrl+Shift+P and search "LGEDV"');
}

export function deactivate() {}
