import * as vscode from 'vscode';
import { exec, spawn } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as http from 'http';
import * as zlib from 'zlib';

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

// Helper: Parse violations from response
function parseViolationsFromResponse(response: string): Array<{severity: string, ruleId?: string, lineNumber?: string, description?: string, originalCode?: string, fixedCode?: string, explanation?: string}> {
  const violations = [];
  
  // Match violations using regex patterns
  const issuePattern = /## 🚨 Issue (\d+): ([^\n]+)\n\*\*Rule Violated:\*\* ([^\n]+)\n\*\*Line[s]?:\*\* ([^\n]+)\n\*\*Severity:\*\* ([^\n]+)/g;
  let match;
  
  while ((match = issuePattern.exec(response)) !== null) {
    const [, issueNum, title, ruleInfo, lineInfo, severity] = match;
    
    // Extract code blocks for this issue
    const issueText = response.substring(match.index, response.indexOf('---', match.index + match[0].length));
    
    const currentCodeMatch = issueText.match(/\*\*Current Code:\*\*\s*```cpp\s*([\s\S]*?)\s*```/);
    const fixedCodeMatch = issueText.match(/\*\*Fixed Code:\*\*\s*```cpp\s*([\s\S]*?)\s*```/);
    const explanationMatch = issueText.match(/\*\*Explanation:\*\*\s*([^\n]+)/);
    
    violations.push({
      severity: severity.trim(),
      ruleId: ruleInfo.split(' - ')[0]?.trim() || `Issue ${issueNum}`,
      lineNumber: lineInfo.trim(),
      description: title.trim(),
      originalCode: currentCodeMatch ? currentCodeMatch[1].trim() : '',
      fixedCode: fixedCodeMatch ? fixedCodeMatch[1].trim() : '',
      explanation: explanationMatch ? explanationMatch[1].trim() : ''
    });
  }
  
  return violations;
}

// Helper: Parse detailed violations (alternative format)
function parseDetailedViolations(response: string): any[] {
  return parseViolationsFromResponse(response);
}

// Helper: Extract complete fixed code
function extractCompleteFixedCode(response: string): string | null {
  const fixedCodeMatch = response.match(/## 🔧 Complete Fixed Code\s*```cpp\s*([\s\S]*?)\s*```/);
  return fixedCodeMatch ? fixedCodeMatch[1].trim() : null;
}

// Helper: Validate rule content patterns
function validateRuleContent(rulesContent: string, ruleType: string, outputChannel: vscode.OutputChannel): {hasLGEDV: boolean, hasMISRA: boolean, hasCERT: boolean, hasRAPIDSCAN: boolean, hasCRITICAL: boolean} {
  const hasLGEDV = /LGEDV_CRCL_\d+/.test(rulesContent);
  const hasMISRA = /Rule \d+-\d+-\d+/.test(rulesContent);
  const hasCERT = /[A-Z]{3}\d{2}-CPP/.test(rulesContent);
  const hasRAPIDSCAN = /RS-\d+/.test(rulesContent);
  const hasCRITICAL = ruleType === 'CRITICAL' && rulesContent.length > 1000;
  
  outputChannel.appendLine(`📋 Rule validation: LGEDV=${hasLGEDV}, MISRA=${hasMISRA}, CERT=${hasCERT}, RapidScan=${hasRAPIDSCAN}, Critical=${hasCRITICAL}`);
  
  return { hasLGEDV, hasMISRA, hasCERT, hasRAPIDSCAN, hasCRITICAL };
}

// Helper: Generate quick view script
// Helper: Extract rule ID from analysis line
function extractRuleId(line: string): string {
  const ruleMatch = line.match(/(LGEDV_CRCL_\d+|Rule \d+-\d+-\d+|[A-Z]{3}\d{2}-CPP|RS-\d+)/);
  return ruleMatch ? ruleMatch[1] : 'Unknown Rule';
}

// Helper: Extract description from analysis line
function extractDescription(line: string): string {
  const descMatch = line.match(/Issue \d+: (.+?)(?:\*\*|$)/);
  return descMatch ? descMatch[1].trim() : 'Rule violation detected';
}

// Helper: Extract severity from analysis line
function extractSeverity(line: string): string | null {
  const severityMatch = line.match(/\*\*Severity:\*\* (Critical|High|Medium|Low)/i);
  return severityMatch ? severityMatch[1] : null;
}

// Helper: Extract line number from analysis line
function extractLineNumber(line: string): string {
  const lineMatch = line.match(/\*\*Line[s]?:\*\* (.+?)(?:\*\*|$)/);
  return lineMatch ? lineMatch[1].trim() : 'Unknown';
}

// Web server for serving HTML reports
async function startWebServerAndOpen(htmlFilePath: string, baseDir?: string, outputChannel?: vscode.OutputChannel): Promise<void> {
  try {
    // Find available port
    const port = await findAvailablePort(8080);
    const directoryToServe = baseDir || path.dirname(htmlFilePath);
    const htmlFileName = path.basename(htmlFilePath);
    
    if (outputChannel) {
      outputChannel.appendLine(`🌐 Starting optimized web server on port ${port}...`);
    }
    
    // Create optimized HTTP server with caching and compression
    const server = http.createServer((req, res) => {
      let filePath = req.url === '/' ? htmlFileName : req.url?.substring(1) || htmlFileName;
      const fullPath = path.join(directoryToServe, filePath);
      
      // Security check - prevent directory traversal
      if (!fullPath.startsWith(directoryToServe)) {
        res.writeHead(403);
        res.end('Forbidden');
        return;
      }
      
      // Add performance headers
      res.setHeader('Cache-Control', 'public, max-age=3600'); // 1 hour cache
      res.setHeader('X-Content-Type-Options', 'nosniff');
      res.setHeader('X-Frame-Options', 'SAMEORIGIN');
      
      fs.readFile(fullPath, (err, data) => {
        if (err) {
          res.writeHead(404, { 'Content-Type': 'text/html' });
          res.end(`
            <!DOCTYPE html>
            <html>
            <head><title>File Not Found</title></head>
            <body>
              <h1>404 - File Not Found</h1>
              <p>The requested file <code>${filePath}</code> was not found.</p>
              <p><a href="/${htmlFileName}">Return to report</a></p>
            </body>
            </html>
          `);
          return;
        }
        
        // Set correct content type and compression
        const ext = path.extname(fullPath).toLowerCase();
        let contentType = 'text/html; charset=utf-8';
        if (ext === '.css') contentType = 'text/css; charset=utf-8';
        else if (ext === '.js') contentType = 'application/javascript; charset=utf-8';
        else if (ext === '.json') contentType = 'application/json; charset=utf-8';
        else if (ext === '.png') contentType = 'image/png';
        else if (ext === '.jpg' || ext === '.jpeg') contentType = 'image/jpeg';
        
        // Add compression for text files
        const shouldCompress = contentType.startsWith('text/') || contentType.includes('javascript') || contentType.includes('json');
        
        if (shouldCompress && req.headers['accept-encoding']?.includes('gzip')) {
          res.setHeader('Content-Encoding', 'gzip');
          res.writeHead(200, { 
            'Content-Type': contentType,
            'Transfer-Encoding': 'chunked'
          });
          zlib.gzip(data, (err: NodeJS.ErrnoException | null, compressed: Buffer) => {
            if (err) {
              res.writeHead(500);
              res.end('Compression error');
              return;
            }
            res.end(compressed);
          });
        } else {
          res.writeHead(200, { 
            'Content-Type': contentType,
            'Content-Length': data.length
          });
          res.end(data);
        }
      });
    });
    
    // Optimize server settings
    server.timeout = 30000; // 30 second timeout
    server.keepAliveTimeout = 5000; // 5 second keep-alive
    server.headersTimeout = 60000; // 60 second headers timeout
    
    server.listen(port, '0.0.0.0', () => { // Listen on all interfaces
      const url = `http://localhost:${port}/${htmlFileName}`;
      
      if (outputChannel) {
        outputChannel.appendLine(`✅ Server running at: ${url}`);
        outputChannel.appendLine(`📂 Serving directory: ${directoryToServe}`);
        outputChannel.appendLine(`⚡ Performance optimizations: Caching, Compression, Keep-Alive`);
        outputChannel.appendLine(`🌐 Network access: http://<your-ip>:${port}/${htmlFileName}`);
      }
      
      // Open in external browser with delay for server to be ready
      setTimeout(() => {
        vscode.env.openExternal(vscode.Uri.parse(url));
      }, 500);
      
      // Show notification with server info
      vscode.window.showInformationMessage(
        `🌐 Optimized HTML Report Server Started`,
        {
          detail: `Server: http://localhost:${port}\nFile: ${htmlFileName}\nOptimizations: Caching + Compression\nNetwork: http://<your-ip>:${port}/${htmlFileName}`,
          modal: false
        }
      );
    });
    
    // Auto-close server after 30 minutes of inactivity
    let lastActivity = Date.now();
    const activityCheck = setInterval(() => {
      if (Date.now() - lastActivity > 30 * 60 * 1000) {
        server.close(() => {
          if (outputChannel) {
            outputChannel.appendLine('🔄 Server auto-closed after 30 minutes of inactivity');
          }
        });
        clearInterval(activityCheck);
      }
    }, 60000); // Check every minute
    
    // Track activity
    server.on('request', () => {
      lastActivity = Date.now();
    });
    
  } catch (error) {
    const errorMsg = error instanceof Error ? error.message : 'Unknown error';
    vscode.window.showErrorMessage(`Failed to start web server: ${errorMsg}`);
    
    if (outputChannel) {
      outputChannel.appendLine(`❌ Web server error: ${errorMsg}`);
      outputChannel.appendLine('💡 Fallback: Opening file directly in VS Code...');
    }
    
    // Fallback to opening in VS Code
    const doc = await vscode.workspace.openTextDocument(htmlFilePath);
    await vscode.window.showTextDocument(doc);
  }
}

// Find available port starting from a base port
async function findAvailablePort(startPort: number): Promise<number> {
  return new Promise((resolve) => {
    const server = http.createServer();
    
    const tryPort = (port: number) => {
      server.listen(port, 'localhost', () => {
        server.close(() => resolve(port));
      }).on('error', () => {
        tryPort(port + 1);
      });
    };
    
    tryPort(startPort);
  });
}

export function activate(context: vscode.ExtensionContext) {
  
  // Helper: Get configuration values
  function getReportDirectory(): string {
    const config = vscode.workspace.getConfiguration('lgedvCodeGuard');
    return config.get<string>('reportDirectory') || '/home/worker/src/codeguard/web_srv/defect_report';
  }

  function getWebServerDirectory(): string {
    const config = vscode.workspace.getConfiguration('lgedvCodeGuard');
    return config.get<string>('webServerDirectory') || '/home/worker/src/codeguard/web_srv';
  }

  function getRuleSetDirectory(): string {
    const config = vscode.workspace.getConfiguration('lgedvCodeGuard');
    const customPath = config.get<string>('ruleSetDirectory');
    return customPath || path.join(__dirname, '..', 'src', 'rule_set');
  }

  function getAutoOpenBrowser(): boolean {
    const config = vscode.workspace.getConfiguration('lgedvCodeGuard');
    return config.get<boolean>('autoOpenBrowser') ?? true;
  }
  
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

  // COMMAND 5: Generate LGEDV Prompt (từ extension_refer.ts)
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
    
    const prompt = `Check for C++ rule violation based on the guidelines in LGEDVRuleGuide.md. If the guidelines file is not existed, please download the file by calling fetch_lgedv_rule from MCP server.
Explain rule content if a rule is violated and show code line number of code that violoate the rule.
Suggest refactored code to comply with these rules.`;

    await openCopilotChatWithPrompt(prompt, 'LGEDV');
  });

  // COMMAND 7: Generate MISRA Prompt
  let generateMisraPromptCommand = vscode.commands.registerCommand('lgedv.generateMisraPrompt', async () => {
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
    
    const prompt = `Check for C++ rule violation based on the guidelines in Misracpp2008Guidelines_en.md. If the guidelines file is not existed, please download the file by calling fetch_misra_rule from MCP server.
Explain rule content if a rule is violated and show code line number of code that violoate the rule.
Suggest refactored code to comply with these rules.`;

    await openCopilotChatWithPrompt(prompt, 'MISRA');
  });

  // COMMAND 8: Generate CERT C++ Prompt
  let generateCertCppPromptCommand = vscode.commands.registerCommand('lgedv.generateCertCppPrompt', async () => {
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
    
    const prompt = `Check for C++ rule violation based on the guidelines in CertcppGuidelines_en.md. If the guidelines file is not existed, please download the file by calling fetch_certcpp_rule from MCP server.
Explain rule content if a rule is violated and show code line number of code that violoate the rule.
Suggest refactored code to comply with these rules.`;

    await openCopilotChatWithPrompt(prompt, 'CERT C++');
  });

  // COMMAND 9: Generate RapidScan Prompt
  let generateRapidScanPromptCommand = vscode.commands.registerCommand('lgedv.generateRapidScanPrompt', async () => {
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
    
    const prompt = `Check for C++ rule violation based on the guidelines in RapidScanGuidelines_en.md. If the guidelines file is not existed, please download the file by calling fetch_rapidScan_rule from MCP server.
Explain rule content if a rule is violated and show code line number of code that violoate the rule.
Suggest refactored code to comply with these rules.`;

    await openCopilotChatWithPrompt(prompt, 'RapidScan');
  });

  // COMMAND 10: Generate Critical Rule Prompt
  let generateCriticalRulePromptCommand = vscode.commands.registerCommand('lgedv.generateCriticalRulePrompt', async () => {
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
    
    const prompt = `Check for C++ rule violation based on the guidelines in CriticalRuleGuideLines.md. If the guidelines file is not existed, please download the file by calling fetch_critical_rule from MCP server.
Explain rule content if a rule is violated and show code line number of code that violoate the rule.
Suggest refactored code to comply with these rules.`;

    await openCopilotChatWithPrompt(prompt, 'Critical Rules');
  });

  // COMMAND 11: Generate HTML Report from existing analysis
  let generateHTMLReportCommand = vscode.commands.registerCommand('lgedv.generateHTMLReport', async () => {
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

    // Ask user to select rule type for HTML generation
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
    
    // Use configured directory instead of hardcoded path
    const defectReportDir = getReportDirectory();
    
    // Check if markdown report exists in configured directory
    const mdReportPath = path.join(defectReportDir, `${baseFileName}-${ruleType.toLowerCase()}-report.md`);
    
    try {
      if (!await fileExists(mdReportPath)) {
        const runAnalysis = await vscode.window.showWarningMessage(
          `No ${ruleType} analysis report found. Do you want to run the analysis first?`,
          'Run Analysis',
          'Cancel'
        );
        
        if (runAnalysis === 'Run Analysis') {
          await runRuleAnalysis(ruleType as 'LGEDV' | 'MISRA' | 'CERT' | 'RAPIDSCAN' | 'CRITICAL');
        }
        return;
      }

      // Read existing markdown report to extract analysis results
      const mdContent = await fs.promises.readFile(mdReportPath, 'utf8');
      const analysisMatch = mdContent.match(/## 📋 Analysis Results\s*\n\n([\s\S]*?)\n\n---/);
      const sourceMatch = mdContent.match(/## 📄 Source Code Analyzed\s*\n\n```cpp\n([\s\S]*?)\n```/);
      
      if (!analysisMatch || !sourceMatch) {
        vscode.window.showErrorMessage('Could not parse existing analysis report');
        return;
      }

      const analysisResults = analysisMatch[1];
      const sourceCode = sourceMatch[1];
      const timestamp = new Date().toISOString();

      // Generate HTML report
      const htmlReport = await generateInteractiveHTMLReport(
        baseFileName, filePath, ruleType, analysisResults, sourceCode, 'GitHub Copilot', timestamp
      );
      
      // Ensure configured defect_report directory exists
      if (!await fileExists(defectReportDir)) {
        await fs.promises.mkdir(defectReportDir, { recursive: true });
      }
      
      const htmlPath = path.join(defectReportDir, `${baseFileName}-${ruleType.toLowerCase()}-report.html`);
      await fs.promises.writeFile(htmlPath, htmlReport, 'utf8');
      
      // Show simple completion notification
      vscode.window.showInformationMessage(
        `✅ HTML report generated: ${path.basename(htmlPath)}`
      );

    } catch (error) {
      vscode.window.showErrorMessage(`Error generating HTML report: ${error}`);
    }
  });

  // COMMAND 12: Open Settings
  let openSettingsCommand = vscode.commands.registerCommand('lgedv.openSettings', async () => {
    await vscode.commands.executeCommand('workbench.action.openSettings', 'lgedvCodeGuard');
  });

  // COMMAND 13: Show Current Configuration
  let showConfigurationCommand = vscode.commands.registerCommand('lgedv.showConfiguration', async () => {
    const reportDir = getReportDirectory();
    const webServerDir = getWebServerDirectory();
    const ruleSetDir = getRuleSetDirectory();
    const autoOpen = getAutoOpenBrowser();

    const message = `📋 **LGEDV CodeGuard Configuration:**

📁 **Report Directory:**
\`${reportDir}\`

🌐 **Web Server Directory:**
\`${webServerDir}\`

📖 **Rule Set Directory:**
\`${ruleSetDir}\`

🚀 **Auto Open Browser:** ${autoOpen ? '✅ Enabled' : '❌ Disabled'}

💡 Use "⚙️ Open Settings" to modify these paths.`;

    await vscode.window.showInformationMessage(
      message,
      { modal: true },
      'Open Settings',
      'Open Report Directory'
    ).then(action => {
      if (action === 'Open Settings') {
        vscode.commands.executeCommand('workbench.action.openSettings', 'lgedvCodeGuard');
      } else if (action === 'Open Report Directory') {
        vscode.commands.executeCommand('vscode.openFolder', vscode.Uri.file(reportDir), { forceNewWindow: false });
      }
    });
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

  // Helper function for opening Copilot Chat with prompt
  async function openCopilotChatWithPrompt(prompt: string, ruleType: string) {
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
        `${ruleType} prompt has been copied to clipboard. Please open Copilot Chat and paste the prompt to check the rules.`
      );
    }
  }

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

      // Save detailed reports (Markdown + HTML + Script)
      const reports = await saveReport(filePath, ruleType, response, codeContent, selectedModel.name, timestamp, outputChannel);
      
      // Show simple completion notification
      vscode.window.showInformationMessage(
        `✅ ${ruleType} analysis complete! HTML report generated in configured directory.`
      );

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
    const ruleSetDir = getRuleSetDirectory(); // Use configured rule set directory
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

  // Helper: Save dual reports (Markdown + HTML)
  async function saveReport(
    filePath: string, 
    ruleType: string, 
    response: string, 
    codeContent: string, 
    modelName: string, 
    timestamp: string, 
    outputChannel: vscode.OutputChannel
  ) {
    const baseFileName = path.basename(filePath, path.extname(filePath));
    
    // Use configured directory instead of hardcoded path
    const defectReportDir = getReportDirectory();
    
    // Ensure the configured defect_report directory exists
    if (!await fileExists(defectReportDir)) {
      await fs.promises.mkdir(defectReportDir, { recursive: true });
      outputChannel.appendLine(`📁 Created configured directory: ${defectReportDir}`);
    } else {
      outputChannel.appendLine(`📁 Using configured directory: ${defectReportDir}`);
    }
    
    // Generate both reports
    outputChannel.appendLine('📝 Generating dual reports (Markdown + HTML)...');
    
    try {
      // 1. Save Markdown Report
      const mdReport = await generateMarkdownReport(baseFileName, filePath, ruleType, response, codeContent, modelName, timestamp);
      const mdPath = path.join(defectReportDir, `${baseFileName}-${ruleType.toLowerCase()}-report.md`);
      await fs.promises.writeFile(mdPath, mdReport, 'utf8');
      outputChannel.appendLine(`✅ Markdown report: ${path.basename(mdPath)} → ${defectReportDir}`);
      
      // 2. Save HTML Report
      const htmlReport = await generateInteractiveHTMLReport(baseFileName, filePath, ruleType, response, codeContent, modelName, timestamp);
      const htmlPath = path.join(defectReportDir, `${baseFileName}-${ruleType.toLowerCase()}-report.html`);
      await fs.promises.writeFile(htmlPath, htmlReport, 'utf8');
      outputChannel.appendLine(`✅ HTML report: ${path.basename(htmlPath)} → ${defectReportDir}`);
      
      outputChannel.appendLine(`📂 All reports saved to configured directory: ${defectReportDir}`);
      
      return { mdPath, htmlPath };
      
    } catch (error) {
      outputChannel.appendLine(`❌ Error saving reports: ${error}`);
      throw error;
    }
  }

  // Helper: Count violations by severity
  function countViolationsBySeverity(violations: Array<{severity: string}>): {critical: number, high: number, medium: number, low: number} {
    return {
      critical: violations.filter(v => v.severity === 'Critical').length,
      high: violations.filter(v => v.severity === 'High').length,
      medium: violations.filter(v => v.severity === 'Medium').length,
      low: violations.filter(v => v.severity === 'Low').length
    };
  }

  // Helper: Generate Enhanced Markdown Report
  async function generateMarkdownReport(
    baseFileName: string,
    filePath: string, 
    ruleType: string, 
    response: string, 
    codeContent: string, 
    modelName: string, 
    timestamp: string
  ): Promise<string> {
    
    // Parse violations from response for better formatting
    const violations = parseViolationsFromResponse(response);
    const severityCounts = countViolationsBySeverity(violations);
    
    return `# 🛡️ ${ruleType} Analysis Report

**File:** \`${path.basename(filePath)}\`  
**Path:** \`${filePath}\`  
**Analyzed:** ${new Date(timestamp).toLocaleString()}  
**Rule Type:** ${ruleType} Coding Standards  
**Model:** ${modelName}  
**Lines of Code:** ${codeContent.split('\n').length}  
**Code Size:** ${codeContent.length} characters  
**Total Issues:** ${violations.length} violations found

---

## 📊 Summary Dashboard

| Severity | Count | Description |
|----------|-------|-------------|
| 🔴 Critical | ${severityCounts.critical} | Must fix immediately |
| 🟠 High | ${severityCounts.high} | Fix before release |
| 🟡 Medium | ${severityCounts.medium} | Improve code quality |
| 🔵 Low | ${severityCounts.low} | Best practice recommendations |

---

## 📋 Analysis Results

${response}

---

## 📄 Source Code Analyzed

\`\`\`cpp
${codeContent}
\`\`\`

---

## 🚀 Quick Actions

To view the interactive HTML report through the configured web server:
\`\`\`bash
cd ${getWebServerDirectory()}
./start_server.sh start
# Then open: http://localhost:8888
\`\`\`

Or view directly:
\`\`\`bash
cd ${getReportDirectory()}
python3 -m http.server 8081
# Then open: http://localhost:8081/${baseFileName}-${ruleType.toLowerCase()}-report.html
\`\`\`

**📂 All reports are saved to:** \`${getReportDirectory()}\`

---

*Generated by LGEDV CodeGuard v1.0.0 - ${ruleType} Analysis*
`;
  }

  // Helper: Generate Interactive HTML Report
  async function generateInteractiveHTMLReport(
    baseFileName: string,
    filePath: string, 
    ruleType: string, 
    response: string, 
    codeContent: string, 
    modelName: string, 
    timestamp: string
  ): Promise<string> {
    
    const violations = parseViolationsFromResponse(response);
    const completeFixedCode = extractCompleteFixedCode(response);
    const severityCounts = countViolationsBySeverity(violations);
    
    return `<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>${ruleType} Code Analysis Report - ${path.basename(filePath)}</title>
    
    <!-- Highlight.js CSS for VS Code theme -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/vs2015.min.css">
    
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6; color: #333;
            background: #f5eee6;
            min-height: 100vh;
        }
        
        .container { max-width: 100%; margin: 0; padding: 20px; width: 100vw; box-sizing: border-box; }
        
        .header {
            background: white; border-radius: 15px; padding: 30px; margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
        }
        
        .header h1 { color: #2c3e50; font-size: 2.5em; margin-bottom: 10px; }
        
        .meta { 
            display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); 
            gap: 20px; margin-top: 20px; 
        }
        
        .meta-item {
            background: rgba(255,255,255,0.1);
            padding: 10px 15px; border-radius: 8px;
            backdrop-filter: blur(10px);
        }
        
        .summary {
            background: white; border-radius: 15px; padding: 30px; margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
        }
        
        .summary h2 { color: #2c3e50; margin-bottom: 20px; }
        
        .severity-grid {
            display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px; margin-top: 20px;
        }
        
        .severity-card {
            background: #f8f9fa; padding: 20px; border-radius: 10px;
            text-align: center; border-left: 4px solid #ddd;
        }
        
        .severity-card.critical { border-left-color: #dc3545; }
        .severity-card.high { border-left-color: #fd7e14; }
        .severity-card.medium { border-left-color: #ffc107; }
        .severity-card.low { border-left-color: #28a745; }
        
        /* Source code section - hidden by default */
        .source-code-section {
            background: #f8f9fa; padding: 20px; border-radius: 10px;
            margin-top: 20px; border: 2px solid #e9ecef;
        }
        
        .source-code-section h3 {
            color: #2c3e50; margin-bottom: 15px;
        }
        
        .source-code-info {
            color: #6c757d; font-size: 0.9em; margin-bottom: 15px;
        }
        
        /* Source code toggle link */
        .source-code-toggle {
            margin-top: 20px; text-align: center;
        }
        
        .source-code-link {
            display: inline-block;
            padding: 12px 20px;
            background: #007acc;
            color: white;
            text-decoration: none;
            border-radius: 8px;
            font-weight: 500;
            transition: background-color 0.3s ease;
        }
        
        .source-code-link:hover {
            background: #005a9e;
            text-decoration: none;
        }
        
        .source-code-link:focus {
            outline: 2px solid #007acc;
            outline-offset: 2px;
        }
        
        /* Line numbers for source code sections */
        .code-with-lines {
            display: flex;
            background: #ffffff;
            border-radius: 8px;
            overflow: hidden;
            /* Height is set dynamically inline */
            width: 100%;
            border: 1px solid #e1e4e8;
        }
        
        .code-with-lines::-webkit-scrollbar {
            width: 8px;
            height: 8px;
        }
        
        .code-with-lines::-webkit-scrollbar-track {
            background: #1e1e1e;
            border-radius: 4px;
        }
        
        .code-with-lines::-webkit-scrollbar-thumb {
            background: #888;
            border-radius: 4px;
        }
        
        .code-with-lines::-webkit-scrollbar-thumb:hover {
            background: #aaa;
        }
        
        .code-with-lines .line-numbers {
            background: #1e1e1e !important;
            color: #858585;
            padding: 16px 12px 16px 16px;
            font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 14px;
            line-height: 1.5;
            user-select: none;
            text-align: right;
            border-right: 1px solid #3e3e42;
            min-width: 50px;
        }
        
        .code-with-lines .line-numbers .line-number {
            display: block;
            height: 1.5em;
        }
        
        .code-with-lines pre {
            margin: 0 !important;
            padding: 16px !important;
            border-radius: 0 !important;
            flex: 1;
            background: #1e1e1e !important;
            border: none !important;
        }
        
        .code-with-lines pre code {
            background: transparent !important;
            padding: 0 !important;
            font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 14px;
            line-height: 1.5;
        }
        
        .source-code-with-lines {
            display: flex;
            background: #ffffff !important;
            border-radius: 8px;
            overflow: hidden;
            /* Height is set dynamically inline */
            width: 100%;
            border: 1px solid #e1e4e8;
        }
        
        .source-code-with-lines::-webkit-scrollbar {
            width: 8px;
            height: 8px;
        }
        
        .source-code-with-lines::-webkit-scrollbar-track {
            background: #ffffff;
            border-radius: 4px;
        }
        
        .source-code-with-lines::-webkit-scrollbar-thumb {
            background: #888;
            border-radius: 4px;
        }
        
        .source-code-with-lines::-webkit-scrollbar-thumb:hover {
            background: #aaa;
        }
        
        .source-code-with-lines .line-numbers {
            background: #f6f8fa !important;
            color: #586069;
            padding: 16px 12px 16px 16px;
            font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 14px;
            line-height: 1.5;
            user-select: none;
            text-align: right;
            border-right: 1px solid #e1e4e8;
            min-width: 50px;
        }
        
        .source-code-with-lines .line-numbers .line-number {
            display: block;
            height: 1.5em;
        }
        
        .source-code-with-lines pre {
            margin: 0 !important;
            padding: 0 !important;
            border-radius: 0 !important;
            flex: 1;
            background: #ffffff !important;
            border: none !important;
            /* Remove max-height to let content determine size */
            overflow: visible;
        }
        
        .source-code-with-lines pre code {
            background: transparent !important;
            padding: 8px 16px 8px 0 !important;
            font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 14px;
            line-height: 1.5;
            display: block;
        }
        
        .content {
            background: white; border-radius: 15px; padding: 30px; margin-bottom: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.1);
        }
        
        .violation {
            border: 1px solid #e9ecef; border-radius: 10px; margin-bottom: 20px;
            overflow: hidden; background: white;
        }
        
        .violation-header {
            background: #f8f9fa; padding: 20px; border-bottom: 1px solid #e9ecef;
        }
        
        .violation-header h3 { margin-bottom: 10px; color: #2c3e50; }
        
        .code-comparison {
            display: grid; grid-template-columns: 1fr 1fr; gap: 20px; padding: 20px;
        }
        
        .code-section h4 {
            color: #2c3e50; margin-bottom: 15px; padding: 10px; 
            background: #ecf0f1; border-radius: 5px;
        }
        
        /* Code styling with highlight.js */
        .code-content { position: relative; }
        
        .code-content pre {
            margin: 0 !important; border-radius: 8px !important;
            /* No fixed height - let content determine height */
            background: #ffffff !important;
            width: 100%;
            border: 1px solid #e1e4e8;
        }
        
        /* Custom scrollbar for webkit browsers - only for long content */
        .code-content pre::-webkit-scrollbar {
            width: 8px;
            height: 8px;
        }
        
        .code-content pre::-webkit-scrollbar-track {
            background: #ffffff;
            border-radius: 4px;
        }
        
        .code-content pre::-webkit-scrollbar-thumb {
            background: #888;
            border-radius: 4px;
        }
        
        .code-content pre::-webkit-scrollbar-thumb:hover {
            background: #aaa;
        }
        
        /* Override highlight.js styling for Light theme */
        .hljs {
            background: #ffffff !important;
            color: #24292e !important;
            font-family: 'Consolas', 'Monaco', 'Courier New', monospace !important;
            font-size: 14px !important;
            line-height: 1.5 !important;
            padding: 0 !important;
            margin: 0 !important;
            border: none !important;
            border-radius: 0 !important;
        }
        
        .code-with-lines .hljs {
            background: #ffffff !important;
            border-radius: 0 !important;
            padding: 0 !important;
            margin: 0 !important;
        }
        
        /* Light theme colors for C++ syntax */
        .hljs-keyword { color: #d73a49 !important; font-weight: bold; }
        .hljs-built_in { color: #005cc5 !important; }
        .hljs-type { color: #005cc5 !important; }
        .hljs-literal { color: #d73a49 !important; }
        .hljs-number { color: #005cc5 !important; }
        .hljs-string { color: #032f62 !important; }
        .hljs-comment { color: #6a737d !important; font-style: italic; }
        .hljs-function { color: #6f42c1 !important; }
        .hljs-variable { color: #e36209 !important; }
        .hljs-operator { color: #24292e !important; }
        .hljs-punctuation { color: #24292e !important; }
        .hljs-class { color: #6f42c1 !important; }
        .hljs-title { color: #6f42c1 !important; }
        .hljs-params { color: #24292e !important; }
        
        /* Line numbers styling */
        .line-numbers {
            background: #f6f8fa !important;
            color: #586069;
            padding: 16px 12px 16px 16px;
            font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 14px;
            line-height: 1.5;
            user-select: none;
            text-align: right;
            border-right: 1px solid #e1e4e8;
            min-width: 50px;
        }
        
        .line-numbers .line-number {
            display: block;
            height: 1.5em;
        }
        
        .code-with-lines pre {
            margin: 0 !important;
            border-radius: 0 !important;
            flex: 1;
            background: #ffffff !important;
            padding: 0 !important;
            /* Remove max-height to let content determine size */
            overflow: visible;
        }
        
        .code-with-lines pre code {
            background: transparent !important;
            padding: 8px 16px 8px 0 !important;
            font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace;
            font-size: 14px;
            line-height: 1.5;
            display: block;
        }
        

        
        /* Code blocks - no collapse functionality */
        
        .source-code-section, .fixed-code-section {
            margin-top: 30px; padding: 20px;
            border-radius: 10px; border: 2px solid;
        }
        
        .fixed-code-section {
            background: linear-gradient(135deg, #e8f5e8, #c8e6c9);
            border-color: #4caf50;
        }
        
        .source-code-section {
            background: linear-gradient(135deg, #f3e5f5, #e1bee7);
            border-color: #9c27b0;
        }
        
        .explanation-box {
            background: linear-gradient(135deg, #e3f2fd, #bbdefb);
            border: 1px solid #2196f3; border-radius: 8px; padding: 15px;
            margin-top: 15px;
        }
        
        /* Gerrit-style side-by-side diff styles */
        .complete-code-comparison {
            margin-top: 30px; padding: 20px;
            border-radius: 10px;
            background: linear-gradient(135deg, #f8f9fa, #e9ecef);
            border: 2px solid #6c757d;
        }
        
        .gerrit-style-diff {
            background: white;
            border: 1px solid #d0d7de;
            border-radius: 8px;
            overflow: hidden;
            margin-top: 15px;
            font-family: 'Consolas', 'Monaco', 'Courier New', monospace;
        }
        
        .diff-header {
            display: grid;
            grid-template-columns: 1fr 1fr;
            background: #f6f8fa;
            border-bottom: 1px solid #d0d7de;
        }
        
        .diff-file-header {
            padding: 12px 16px;
            font-weight: 600;
            font-size: 14px;
            border-right: 1px solid #d0d7de;
            text-align: center;
        }
        
        .diff-file-header:last-child {
            border-right: none;
        }
        
        .diff-file-header.original {
            background: #fff1f0;
            color: #d73a49;
            border-right: 1px solid #d0d7de;
        }
        
        .diff-file-header.fixed {
            background: #f0fff4;
            color: #28a745;
        }
        
        .diff-content {
            display: grid;
            grid-template-columns: 1fr 1fr;
            align-items: start;
            overflow: hidden;
        }
        
        .diff-side {
            position: relative;
            overflow: auto;
            border-right: 1px solid #d0d7de;
            scrollbar-width: thin;
            scrollbar-color: #888 #1e1e1e;
        }
        
        .diff-side::-webkit-scrollbar {
            width: 8px;
            height: 8px;
        }
        
        .diff-side::-webkit-scrollbar-track {
            background: #1e1e1e;
            border-radius: 4px;
        }
        
        .diff-side::-webkit-scrollbar-thumb {
            background: #888;
            border-radius: 4px;
        }
        
        .diff-side::-webkit-scrollbar-thumb:hover {
            background: #aaa;
        }
        
        .diff-side:last-child {
            border-right: none;
        }
        
        .diff-side.original {
            background: #ffeef0;
        }
        
        .diff-side.fixed {
            background: #f0fff4;
        }
        
        .diff-side .code-content {
            height: 100%;
            position: relative;
            overflow: auto;
        }
        
        .diff-side .code-content .code-with-lines {
            height: auto;
            background: #ffffff !important;
            border-radius: 0 !important;
            max-height: none;
            overflow: visible;
        }
        
        .diff-side .code-content .line-numbers {
            background: #f6f8fa !important;
            color: #586069 !important;
            border-right: 1px solid #e1e4e8 !important;
            position: sticky;
            left: 0;
            z-index: 1;
        }
        
        .diff-side .code-content pre {
            margin: 0 !important;
            border-radius: 0 !important;
            height: auto;
            max-height: none !important;
            white-space: pre;
            word-wrap: break-word;
            font-size: 14px;
            line-height: 1.4;
            padding: 16px;
            overflow: visible;
        }
        
        /* Match the violation sections' code highlighting style */
        .diff-side .hljs {
            background: #ffffff !important;
            color: #24292e !important;
            font-family: 'Consolas', 'Monaco', 'Courier New', monospace !important;
            font-size: 14px !important;
            line-height: 1.45 !important;
            padding: 16px !important;
            border-radius: 0 !important;
        }
        
        /* Gerrit-style line-by-line alignment */
        .diff-line {
            display: block;
            min-height: 1.45em;
            padding: 0 8px;
            border-left: 3px solid transparent;
        }
        
        .diff-line.removed {
            background-color: #ffeef0;
            border-left-color: #f85149;
        }
        
        .diff-line.added {
            background-color: #f0fff4;
            border-left-color: #3fb950;
        }
        
        .diff-line.context {
            background-color: transparent;
        }
        
        /* Line numbers for better alignment */
        .diff-line-number {
            display: inline-block;
            width: 40px;
            text-align: right;
            color: #656d76;
            font-size: 12px;
            margin-right: 8px;
            user-select: none;
        }
        
        @media (max-width: 768px) {
            .code-comparison { grid-template-columns: 1fr; }
            .meta { grid-template-columns: 1fr; }
            .severity-grid { grid-template-columns: 1fr; }
            
            /* Stack diff sides vertically on mobile */
            .diff-header {
                grid-template-columns: 1fr;
            }
            
            .diff-content {
                grid-template-columns: 1fr;
            }
            
            .diff-side {
                border-right: none;
                border-bottom: 1px solid #d0d7de;
            }
            
            .diff-side:last-child {
                border-bottom: none;
            }
            
            .diff-file-header {
                border-right: none;
                border-bottom: 1px solid #d0d7de;
            }
            
            .diff-file-header:last-child {
                border-bottom: none;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🛡️ ${ruleType} Code Analysis Report</h1>
            <div class="meta">
                <div class="meta-item"><strong>📁 File:</strong> ${path.basename(filePath)}</div>
                <div class="meta-item"><strong>📍 Path:</strong> ${filePath}</div>
                <div class="meta-item"><strong>⏰ Generated:</strong> ${new Date(timestamp).toLocaleString()}</div>
                <div class="meta-item"><strong>🤖 Analyzer:</strong> ${modelName}</div>
                <div class="meta-item"><strong>🎯 Rule Set:</strong> ${ruleType}</div>
                <div class="meta-item"><strong>🔍 Total Issues:</strong> ${violations.length}</div>
            </div>
        </div>

        <div class="summary">
            <h2>📊 Analysis Summary</h2>
            <div class="severity-grid">
                <div class="severity-card critical">
                    <h3 style="color: #dc3545;">🔴 Critical</h3>
                    <div style="font-size: 2em; font-weight: bold; margin: 10px 0;">${severityCounts.critical || 0}</div>
                    <p>Must fix immediately</p>
                </div>
                <div class="severity-card high">
                    <h3 style="color: #fd7e14;">🟠 High</h3>
                    <div style="font-size: 2em; font-weight: bold; margin: 10px 0;">${severityCounts.high || 0}</div>
                    <p>Fix before release</p>
                </div>
                <div class="severity-card medium">
                    <h3 style="color: #ffc107;">🟡 Medium</h3>
                    <div style="font-size: 2em; font-weight: bold; margin: 10px 0;">${severityCounts.medium || 0}</div>
                    <p>Improve code quality</p>
                </div>
                <div class="severity-card low">
                    <h3 style="color: #28a745;">🔵 Low</h3>
                    <div style="font-size: 2em; font-weight: bold; margin: 10px  0;">${severityCounts.low || 0}</div>
                    <p>Best practice recommendations</p>
                </div>
            </div>
            
            <!-- Original Source Code Section -->
            <div class="source-code-toggle">
                <a href="#" onclick="toggleSourceCode(); return false;" class="source-code-link">
                    📄 View Original Source Code
                </a>
            </div>
            <div class="source-code-section" id="source-code-section" style="display: none;">
                <h3>📄 Original Source Code</h3>
                <div class="source-code-info">
                    <strong>File:</strong> ${path.basename(filePath)} | 
                    <strong>Lines:</strong> ${codeContent.split('\n').length} | 
                    <strong>Size:</strong> ${Math.round(codeContent.length / 1024 * 100) / 100} KB
                </div>
                <div class="code-content">
                    ${createCodeWithLines(codeContent)}
                </div>
            </div>
        </div>

        <div class="content">
            <h2>🔍 Detailed Violations</h2>
            
            ${violations.map((violation, index) => {
                const getSeverityColor = (severity: string): string => {
                    switch (severity?.toLowerCase()) {
                        case 'critical': return '#dc3545';
                        case 'high': return '#fd7e14';
                        case 'medium': return '#ffc107';
                        case 'low': return '#28a745';
                        default: return '#6c757d';
                    }
                };
                
                return `
                <div class="violation">
                    <div class="violation-header">
                        <h3>🚫 Violation ${index + 1}: ${violation.ruleId || 'Rule Violation'} - ${violation.description}</h3>
                        <p><strong>📍 Line:</strong> ${violation.lineNumber || 'Multiple lines'}</p>
                        <p><strong>📝 Description:</strong> ${violation.description || 'No description provided'}</p>
                        <p><strong>⚠️ Severity:</strong> <span style="color: ${getSeverityColor(violation.severity)}; font-weight: bold;">${violation.severity || 'Unknown'}</span></p>
                    </div>
                    
                    <div class="gerrit-style-diff">
                        <div class="diff-header">
                            <div class="diff-file-header original">❌ Current Code (Violating)</div>
                            <div class="diff-file-header fixed">✅ Fixed Code (Compliant)</div>
                        </div>
                        <div class="diff-content">
                            <div class="diff-side original">
                                <div class="code-content">
                                    ${createCodeWithLines(violation.originalCode || 'No code provided')}
                                </div>
                            </div>
                            <div class="diff-side fixed">
                                <div class="code-content">
                                    ${createCodeWithLines(violation.fixedCode || 'No fix provided')}
                                </div>
                            </div>
                        </div>
                    </div>
                    
                    ${violation.explanation ? `
                        <div class="explanation-box">
                            <h4>💡 Explanation</h4>
                            <p>${violation.explanation}</p>
                        </div>
                    ` : ''}
                </div>
                `;
            }).join('')}

            ${completeFixedCode ? `
                <div class="complete-code-comparison">
                    <h2>🔄 Complete Code Comparison</h2>
                    <p>Side-by-side comparison of the original and refactored code (Gerrit-style diff):</p>
                    <div class="gerrit-style-diff">
                        <div class="diff-header">
                            <div class="diff-file-header original">📄 Original Source Code</div>
                            <div class="diff-file-header fixed">✨ Complete Fixed Code</div>
                        </div>
                        <div class="diff-content">
                            <div class="diff-side original">
                                <div class="code-content">
                                    ${createCodeWithLines(codeContent)}
                                </div>
                            </div>
                            <div class="diff-side fixed">
                                <div class="code-content">
                                    ${createCodeWithLines(completeFixedCode)}
                                </div>
                            </div>
                        </div>
                    </div>
                </div>
            ` : `
                <div class="source-code-toggle">
                    <a href="#" onclick="toggleSourceCode(); return false;" class="source-code-link">
                        📄 View Original Source Code
                    </a>
                </div>
                <div class="source-code-section" id="source-code-section-2" style="display: none;">
                    <h2>📄 Original Source Code</h2>
                    <p>Original source code that was analyzed:</p>
                    <div class="code-content">
                        ${createCodeWithLines(codeContent)}
                    </div>
                </div>
            `}
        </div>
    </div>

    <!-- Highlight.js JavaScript -->
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/cpp.min.js"></script>

    <script>
        // Initialize highlight.js
        document.addEventListener('DOMContentLoaded', function() {
            hljs.highlightAll();
            console.log('✅ Highlight.js initialized - syntax highlighting applied');
        });
        
        // Toggle source code sections visibility
        function toggleSourceCode() {
            const sections = document.querySelectorAll('#source-code-section, #source-code-section-2');
            const links = document.querySelectorAll('.source-code-link');
            
            sections.forEach((section, index) => {
                if (section) {
                    const isHidden = section.style.display === 'none';
                    section.style.display = isHidden ? 'block' : 'none';
                    
                    // Update link text
                    const link = links[index];
                    if (link) {
                        link.textContent = isHidden ? '📄 Hide Original Source Code' : '📄 View Original Source Code';
                    }
                }
            });
        }
    </script>
</body>
</html>`;
  }

  // Helper: Create Gerrit-style line-by-line diff
  function createLineByLineDiff(originalCode: string, fixedCode: string): {leftSide: string, rightSide: string} {
    const originalLines = originalCode.split('\n');
    const fixedLines = fixedCode.split('\n');
    
    // Pad shorter array to match lengths for proper alignment
    const maxLines = Math.max(originalLines.length, fixedLines.length);
    while (originalLines.length < maxLines) originalLines.push('');
    while (fixedLines.length < maxLines) fixedLines.push('');
    
    let leftSide = '';
    let rightSide = '';
    
    for (let i = 0; i < maxLines; i++) {
      const leftLine = originalLines[i] || '';
      const rightLine = fixedLines[i] || '';
      const lineNum = i + 1;
      
      // Determine line type for styling
      let leftClass = 'context';
      let rightClass = 'context';
      
      if (leftLine !== rightLine) {
        if (leftLine && !rightLine) {
          leftClass = 'removed';
          rightClass = 'context';
        } else if (!leftLine && rightLine) {
          leftClass = 'context';
          rightClass = 'added';
        } else {
          leftClass = 'removed';
          rightClass = 'added';
        }
      }
      
      // Escape HTML
      const escapedLeft = leftLine.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
      const escapedRight = rightLine.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
      
      leftSide += `<span class="diff-line ${leftClass}"><span class="diff-line-number">${lineNum}</span>${escapedLeft}</span>\n`;
      rightSide += `<span class="diff-line ${rightClass}"><span class="diff-line-number">${lineNum}</span>${escapedRight}</span>\n`;
    }
    
    return { leftSide, rightSide };
  }

  // Helper: Create code block with line numbers
  function createCodeWithLines(code: string, language: string = 'cpp'): string {
    const lines = code.split('\n');
    
    // Remove any leading empty line that might cause alignment issues
    if (lines.length > 0 && lines[0].trim() === '') {
      lines.shift();
    }
    
    // Remove trailing empty lines to avoid extra space
    while (lines.length > 0 && lines[lines.length - 1].trim() === '') {
      lines.pop();
    }
    
    const lineNumbers = lines.map((_, index) => 
      `<span class="line-number">${index + 1}</span>`
    ).join('\n');
    
    // We'll let the client-side highlight.js handle the syntax highlighting
    const escapedCode = lines.map(line => 
      line.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
    ).join('\n');
    
    return `<div class="code-with-lines"><div class="line-numbers">${lineNumbers}</div><pre><code class="language-${language}">${escapedCode}</code></pre></div>`;
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
    generateMisraPromptCommand,
    generateCertCppPromptCommand,
    generateRapidScanPromptCommand,
    generateCriticalRulePromptCommand,
    generateHTMLReportCommand,
    openSettingsCommand,
    showConfigurationCommand,
    checkStaticAnalysisCommand
  );
}

export function deactivate() {}
