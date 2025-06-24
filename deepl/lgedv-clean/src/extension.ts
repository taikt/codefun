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

  // COMMAND 2: Check MISRA Rules Only  
  let checkMISRACommand = vscode.commands.registerCommand('lgedv.checkMISRARules', async () => {
    await runRuleAnalysis('MISRA');
  });

  // COMMAND 3: Check CERT Rules Only
  let checkCERTCommand = vscode.commands.registerCommand('lgedv.checkCERTRules', async () => {
    await runRuleAnalysis('CERT');
  });

  // COMMAND 4: Check RapidScan Rules Only
  let checkRapidScanCommand = vscode.commands.registerCommand('lgedv.checkRapidScanRules', async () => {
    await runRuleAnalysis('RAPIDSCAN');
  });

  // COMMAND 5: Check Critical Rules (Combined)
  let checkAllRulesCommand = vscode.commands.registerCommand('lgedv.checkAllRules', async () => {
    await runRuleAnalysis('CRITICAL');
  });

  // COMMAND 6: Interactive Rule Selection
  let interactiveCheckCommand = vscode.commands.registerCommand('lgedv.interactiveCheck', async () => {
    const choice = await vscode.window.showQuickPick([
      {
        label: '🎯 LGEDV Rules Only',
        description: 'Check only LGEDV_CRCL rules (fast, focused)',
        detail: 'Best for LGEDV compliance check - faster analysis'
      },
      {
        label: '🛡️ MISRA C++ Rules Only', 
        description: 'Check only MISRA C++ 2008 rules (fast, focused)',
        detail: 'Best for MISRA compliance check - faster analysis'
      },
      {
        label: '� CERT C++ Rules Only', 
        description: 'Check only CERT C++ rules (fast, focused)',
        detail: 'Best for CERT compliance check - faster analysis'
      },
      {
        label: '⚡ RapidScan Rules Only', 
        description: 'Check only RapidScan rules (fast, focused)',
        detail: 'Best for RapidScan compliance check - faster analysis'
      },
      {
        label: '🔍 Critical Rules Combined',
        description: 'Check critical LGEDV rules (most important)', 
        detail: 'Focused analysis on critical compliance rules'
      }
    ], {
      placeHolder: 'Select which rules to check against your code',
      title: 'LGEDV CodeGuard - Choose Analysis Type'
    });

    if (choice) {
      if (choice.label.includes('LGEDV')) {
        await runRuleAnalysis('LGEDV');
      } else if (choice.label.includes('MISRA')) {
        await runRuleAnalysis('MISRA');
      } else if (choice.label.includes('CERT')) {
        await runRuleAnalysis('CERT');
      } else if (choice.label.includes('RapidScan')) {
        await runRuleAnalysis('RAPIDSCAN');
      } else {
        await runRuleAnalysis('CRITICAL');
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
      const { hasLGEDV, hasMISRA, hasCERT, hasRAPIDSCAN, hasCRITICAL } = validateRuleContent(rulesContent, ruleType, outputChannel);
      
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
  async function loadRules(ruleType: 'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL', outputChannel: vscode.OutputChannel): Promise<string> {
    const ruleSetDir = path.join(__dirname, '..', 'src', 'rule_set'); // Đọc trực tiếp từ rule_set
    let rulesContent = '';

    outputChannel.appendLine(`📂 Rule set directory: ${ruleSetDir}`);

    try {
      // LGEDV Rules
      if (ruleType === 'LGEDV') {
        const lgedvContent = await loadRuleFile('LGEDVRuleGuide.md', ruleSetDir, outputChannel);
        if (lgedvContent) {
          rulesContent += `# LGEDV Rules\n${lgedvContent}\n\n`;
          outputChannel.appendLine('✅ LGEDV rules loaded');
        } else {
          outputChannel.appendLine('⚠️ LGEDV rules not found');
        }
      }

      // MISRA Rules
      if (ruleType === 'MISRA') {
        const misraContent = await loadRuleFile('Misracpp2008Guidelines_en.md', ruleSetDir, outputChannel);
        if (misraContent) {
          rulesContent += `# MISRA C++ 2008 Rules\n${misraContent}\n\n`;
          outputChannel.appendLine('✅ MISRA rules loaded');
        } else {
          outputChannel.appendLine('⚠️ MISRA rules not found');
        }
      }

      // CERT Rules
      if (ruleType === 'CERT') {
        const certContent = await loadRuleFile('CertcppGuidelines_en.md', ruleSetDir, outputChannel);
        if (certContent) {
          rulesContent += `# CERT C++ Rules\n${certContent}\n\n`;
          outputChannel.appendLine('✅ CERT rules loaded');
        } else {
          outputChannel.appendLine('⚠️ CERT rules not found');
        }
      }

      // RapidScan Rules
      if (ruleType === 'RAPIDSCAN') {
        const rapidScanContent = await loadRuleFile('RapidScanGuidelines_en.md', ruleSetDir, outputChannel);
        if (rapidScanContent) {
          rulesContent += `# RapidScan Rules\n${rapidScanContent}\n\n`;
          outputChannel.appendLine('✅ RapidScan rules loaded');
        } else {
          outputChannel.appendLine('⚠️ RapidScan rules not found');
        }
      }

      // Critical Rules (replaces ALL)
      if (ruleType === 'CRITICAL') {
        const criticalContent = await loadRuleFile('CriticalRuleGuideLines.md', ruleSetDir, outputChannel);
        if (criticalContent) {
          rulesContent += `# Critical Rules\n${criticalContent}\n\n`;
          outputChannel.appendLine('✅ Critical rules loaded');
        } else {
          outputChannel.appendLine('⚠️ Critical rules not found');
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
      
      // Optimize prompt size for single rule type vs critical
      const maxSize = ruleType === 'CRITICAL' ? 30000 : 40000; // More space for single rule type
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

  // Helper: Load individual rule file
  async function loadRuleFile(fileName: string, ruleSetDir: string, outputChannel: vscode.OutputChannel): Promise<string | null> {
    const filePath = path.join(ruleSetDir, fileName);

    outputChannel.appendLine(`🔍 Loading ${fileName} from: ${filePath}`);

    try {
      if (await fileExists(filePath)) {
        outputChannel.appendLine(`✅ Found ${fileName}`);
        return await fs.promises.readFile(filePath, 'utf8');
      } else {
        outputChannel.appendLine(`❌ File not found: ${fileName}`);
      }
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
    if (ruleType === 'CRITICAL') {
      // For Critical rules, optimize specifically for critical content
      const criticalMatch = rulesContent.match(/# Critical Rules\n([\s\S]*?)(?=\n# |$)/);
      
      if (criticalMatch) {
        return `# Critical Rules\n${criticalMatch[1].substring(0, maxSize)}\n\n`;
      }
    }
    
    // For single rule type, simple truncation
    return rulesContent.substring(0, maxSize);
  }

  // Helper: Create focused prompt based on rule type
  function createFocusedPrompt(codeContent: string, rulesContent: string, ruleType: 'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL'): string {
    const ruleTypeDescription: Record<'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL', string> = {
      'LGEDV': 'LGEDV_CRCL rules for automotive code compliance',
      'MISRA': 'MISRA C++ 2008 rules for safety-critical software',
      'CERT': 'CERT C++ Secure Coding Standard rules',
      'RAPIDSCAN': 'RapidScan static analysis rules',
      'CRITICAL': 'Critical LGEDV rules for essential code compliance'
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
- Focus specifically on ${ruleType === 'CRITICAL' ? 'CRITICAL' : ruleType} rule violations
- Cite EXACT rule numbers (e.g., LGEDV_CRCL_0001, Rule 8-4-3, DCL50-CPP, RS-001)
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
  function validateRuleContent(rulesContent: string, ruleType: 'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL', outputChannel: vscode.OutputChannel): { hasLGEDV: boolean, hasMISRA: boolean, hasCERT: boolean, hasRAPIDSCAN: boolean, hasCRITICAL: boolean } {
    // Detect LGEDV rules by pattern
    const lgedvPatterns = [
      /LGEDV_CRCL_\d+/,           // LGEDV_CRCL_0001, LGEDV_CRCL_0013, etc.
      /Rule LGEDV_CRCL_/,         // Alternative format
      /## Rule LGEDV_CRCL_/       // Markdown header format
    ];
    
    // Detect MISRA rules by pattern
    const misraPatterns = [
      /Rule \d+-\d+-\d+/,         // Rule 6-4-2, Rule 8-5-1, etc.
      /Rule MISRA \d+-\d+-\d+/,   // Rule MISRA 6-4-2, Rule MISRA 8-5-1, etc.
      /### Rule \d+-\d+-\d+/,     // Markdown header format
      /## Rule MISRA \d+-\d+-\d+/, // ## Rule MISRA 6-4-2 format
      /MISRA.*Rule.*\d+-\d+-\d+/, // Various MISRA rule formats
      /MISRA-C\+\+.*\d+-\d+-\d+/, // MISRA-C++ specific
      /\*\*Rule \d+-\d+-\d+\*\*/  // Bold rule format
    ];

    // Detect CERT rules by pattern
    const certPatterns = [
      /CERT.*C\+\+/,              // CERT C++ references
      /SEI.*CERT/,                // SEI CERT references
      /CERT-.*-\d+/,              // CERT rule format
      /DCL\d+/,                   // CERT DCL rules
      /EXP\d+/,                   // CERT EXP rules
      /CTR\d+/,                   // CERT CTR rules
      /ARR\d+/,                   // CERT ARR rules
      /MEM\d+/                    // CERT MEM rules
    ];

    // Detect RapidScan rules by pattern
    const rapidScanPatterns = [
      /RapidScan/i,               // RapidScan references
      /Rapid.*Scan/i,             // Alternative format
      /RAPID.*SCAN/,              // Uppercase format
      /RS-\d+/,                   // RapidScan rule format
      /### RS-/                   // Markdown header format
    ];

    // Detect Critical rules by pattern
    const criticalPatterns = [
      /## Rule LGEDV_CRCL_/,      // Critical LGEDV rules format
      /## Rule MISRA \d+-\d+-\d+/, // Critical MISRA rules format
      /LGEDV_CRCL_\d+/,           // LGEDV rule numbers
      /Rule MISRA \d+-\d+-\d+/,   // MISRA rule numbers
      /Critical.*Rule/i,          // Critical rule references
      /CRITICAL/i                 // Critical keyword
    ];
    
    // Check patterns
    const hasLGEDV = lgedvPatterns.some(pattern => pattern.test(rulesContent));
    const hasMISRA = misraPatterns.some(pattern => pattern.test(rulesContent));
    const hasCERT = certPatterns.some(pattern => pattern.test(rulesContent));
    const hasRAPIDSCAN = rapidScanPatterns.some(pattern => pattern.test(rulesContent));
    const hasCRITICAL = criticalPatterns.some(pattern => pattern.test(rulesContent));
    
    // Detailed logging
    outputChannel.appendLine(`🔍 Rule validation:`);
    outputChannel.appendLine(`   LGEDV rules detected: ${hasLGEDV ? '✅' : '❌'}`);
    outputChannel.appendLine(`   MISRA rules detected: ${hasMISRA ? '✅' : '❌'}`);
    outputChannel.appendLine(`   CERT rules detected: ${hasCERT ? '✅' : '❌'}`);
    outputChannel.appendLine(`   RapidScan rules detected: ${hasRAPIDSCAN ? '✅' : '❌'}`);
    outputChannel.appendLine(`   Critical rules detected: ${hasCRITICAL ? '✅' : '❌'}`);
    
    // Show rule counts if detected
    if (hasLGEDV) {
      const lgedvCount = (rulesContent.match(/LGEDV_CRCL_\d+/g) || []).length;
      outputChannel.appendLine(`   LGEDV rules found: ${lgedvCount}`);
    }
    
    if (hasMISRA) {
      const misraCount1 = (rulesContent.match(/Rule \d+-\d+-\d+/g) || []).length;
      const misraCount2 = (rulesContent.match(/Rule MISRA \d+-\d+-\d+/g) || []).length;
      const totalMisraCount = misraCount1 + misraCount2;
      outputChannel.appendLine(`   MISRA rules found: ${totalMisraCount}`);
    }

    if (hasCERT) {
      const certCount = (rulesContent.match(/DCL\d+|EXP\d+|CTR\d+|ARR\d+|MEM\d+/g) || []).length;
      outputChannel.appendLine(`   CERT rules found: ${certCount}`);
    }

    if (hasRAPIDSCAN) {
      const rapidScanCount = (rulesContent.match(/RS-\d+/g) || []).length;
      outputChannel.appendLine(`   RapidScan rules found: ${rapidScanCount}`);
    }

    if (hasCRITICAL) {
      const lgedvCriticalCount = (rulesContent.match(/## Rule LGEDV_CRCL_/g) || []).length;
      const misraCriticalCount = (rulesContent.match(/## Rule MISRA \d+-\d+-\d+/g) || []).length;
      const totalCriticalCount = lgedvCriticalCount + misraCriticalCount;
      outputChannel.appendLine(`   Critical rules found: ${totalCriticalCount} (${lgedvCriticalCount} LGEDV + ${misraCriticalCount} MISRA)`);
    }
    
    // Validation warnings
    if (ruleType === 'LGEDV' && !hasLGEDV) {
      outputChannel.appendLine('⚠️ Warning: LGEDV analysis requested but no LGEDV rules detected');
    }
    
    if (ruleType === 'MISRA' && !hasMISRA) {
      outputChannel.appendLine('⚠️ Warning: MISRA analysis requested but no MISRA rules detected');
    }

    if (ruleType === 'CERT' && !hasCERT) {
      outputChannel.appendLine('⚠️ Warning: CERT analysis requested but no CERT rules detected');
    }

    if (ruleType === 'RAPIDSCAN' && !hasRAPIDSCAN) {
      outputChannel.appendLine('⚠️ Warning: RapidScan analysis requested but no RapidScan rules detected');
    }

    if (ruleType === 'CRITICAL' && !hasCRITICAL) {
      outputChannel.appendLine('⚠️ Warning: Critical analysis requested but no Critical rules detected');
    }
    
    return { hasLGEDV, hasMISRA, hasCERT, hasRAPIDSCAN, hasCRITICAL };
  }

  // Register all commands
  context.subscriptions.push(
    checkLGEDVCommand, 
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
