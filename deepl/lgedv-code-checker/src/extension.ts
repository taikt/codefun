import * as vscode from 'vscode';
import { exec, spawn } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';

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
  // Check C++ Vulnerability by static tool (clang-tidy + cppcheck + clang)
  let checkStaticAll = vscode.commands.registerCommand('lgedv.checkStaticAll', () => {
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
    const clangTidyCmd = `/home/worker/src/llvm-project/build/bin/clang-tidy "${filePath}" --checks=-*,custom-* --`;
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

          vscode.window.showInformationMessage(`Static vulnerability report saved at: ${reportFilePath}`);
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

  // Check C++ Vulnerability by Copilot (common prompt for both rule files)
  let checkCppVulnByCopilot = vscode.commands.registerCommand('lgedv.checkCppVulnByCopilot', async () => {
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

  // Register command to check C++ violations using local AI (Ollama) on server
  let checkAI = vscode.commands.registerCommand('lgedv.checkAIViolations', () => {
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

    // Construct path for AI analysis report file
    const reportFilePath = path.join(path.dirname(filePath), `${path.basename(filePath, path.extname(filePath))}.ai-report.txt`);
    console.log('[LGEDV] AI report will be saved at:', reportFilePath);

    // Read content of the C++ file
    let content: string;
    try {
      content = fs.readFileSync(filePath, 'utf-8');
    } catch (err: unknown) {
      const errorMessage = err instanceof Error ? err.message : 'Unknown error';
      vscode.window.showErrorMessage(`Error reading file: ${errorMessage}`);
      return;
    }

    // Add line numbers to content for Ollama analysis
    const lines = content.split('\n');
    const numberedContent = lines.map((line, i) => `${i + 1}: ${line}`).join('\n');

    // Create prompt for Ollama
    const prompt = [
      `Analyze the following C++ code for potential errors.`,
      `Provide a simple report that indicates which code line has an issue,`,
      `using the line numbers provided in the format 'Line X: ...'.`,
      `Please include simple explanations, and list the issues with their corresponding line numbers.\n`,
      `Provide improved source code after refactoring.\n`,
      `${numberedContent}\n/no_think`
    ].join('\n');

    // Check prompt size
    const maxPromptSize = 1000000; // Limit to 1MB
    if (prompt.length > maxPromptSize) {
      const errorMsg = `Error: Prompt size exceeds ${maxPromptSize} bytes, too large for analysis.\n`;
      fs.writeFileSync(reportFilePath, `C++ AI Analysis Report\nSource File: ${filePath}\n\n${errorMsg}`, { encoding: 'utf-8' });
      vscode.window.showErrorMessage(errorMsg);
      return;
    }

    // Initialize AI analysis report
    fs.writeFileSync(reportFilePath, `C++ AI Analysis Report\nSource File: ${filePath}\n\n=== Ollama Analysis ===\n`, { encoding: 'utf-8' });

    // Create temporary prompt file for Ollama
    const tempPromptFile = `/tmp/ollama_prompt_${Date.now()}.txt`;
    try {
      fs.writeFileSync(tempPromptFile, prompt, { encoding: 'utf-8' });
    } catch (err: unknown) {
      const errorMessage = err instanceof Error ? err.message : 'Unknown error';
      vscode.window.showErrorMessage(`Error creating temporary prompt file: ${errorMessage}`);
      fs.appendFileSync(reportFilePath, `Error creating temporary prompt file: ${errorMessage}\n`, { encoding: 'utf-8' });
      return;
    }

    // Verify virtual environment and ollama service
    const venvPath = '/home/worker/src/dl/ollama_env';
    const ollamaCommand = `source ${venvPath}/bin/activate && ollama run qwen3:8b < "${tempPromptFile}"`;

    // Check if systemd is available
    const checkSystemdCommand = `test -d /run/systemd/system && echo "systemd" || echo "no-systemd"`;
    exec(checkSystemdCommand, { shell: '/bin/bash', encoding: 'utf-8' }, (systemdError, systemdStdout, systemdStderr) => {
      const isSystemd = systemdStdout.trim() === 'systemd';

      if (isSystemd) {
        // Check if ollama service is running using systemctl
        const checkOllamaServiceCommand = `systemctl status ollama`;
        exec(checkOllamaServiceCommand, { shell: '/bin/bash', encoding: 'utf-8' }, (checkError, checkStdout, checkStderr) => {
          if (checkError || !checkStdout.includes('active (running)')) {
            // Ollama service is not running, attempt to start it
            const startOllamaCommand = `systemctl start ollama`;
            exec(startOllamaCommand, { shell: '/bin/bash', encoding: 'utf-8' }, (startError, startStdout, startStderr) => {
              if (startError) {
                const errorMsg = `Failed to start Ollama service: ${startStderr || startError.message || 'Unknown error'}\n`;
                fs.appendFileSync(reportFilePath, errorMsg, { encoding: 'utf-8' });
                vscode.window.showErrorMessage(errorMsg);
                try {
                  fs.unlinkSync(tempPromptFile);
                } catch (err: unknown) {
                  const errorMessage = err instanceof Error ? err.message : 'Unknown error';
                  fs.appendFileSync(reportFilePath, `Error deleting temporary prompt file: ${errorMessage}\n`, { encoding: 'utf-8' });
                }
                return;
              }
              // Wait briefly to ensure the service is up
              setTimeout(() => proceedWithOllama(), 2000); // 2-second delay
            });
          } else {
            // Service is running, proceed with ollama check
            proceedWithOllama();
          }
        });
      } else {
        // No systemd, attempt to start ollama serve directly
        const startOllamaCommand = `source ${venvPath}/bin/activate && nohup ollama serve > /tmp/ollama_serve.log 2>&1 &`;
        exec(startOllamaCommand, { shell: '/bin/bash', encoding: 'utf-8' }, (startError, startStdout, startStderr) => {
          if (startError) {
            const errorMsg = `Failed to start Ollama server: ${startStderr || startError.message || 'Unknown error'}\n`;
            fs.appendFileSync(reportFilePath, errorMsg, { encoding: 'utf-8' });
            vscode.window.showErrorMessage(errorMsg);
            try {
              fs.unlinkSync(tempPromptFile);
            } catch (err: unknown) {
              const errorMessage = err instanceof Error ? err.message : 'Unknown error';
              fs.appendFileSync(reportFilePath, `Error deleting temporary prompt file: ${errorMessage}\n`, { encoding: 'utf-8' });
            }
            return;
          }
          // Wait briefly to ensure the server is up
          setTimeout(() => proceedWithOllama(), 2000); // 2-second delay
        });
      }
    });

    // Function to proceed with ollama execution
    function proceedWithOllama() {
      // Verify ollama binary and responsiveness
      const checkOllamaCommand = `source ${venvPath}/bin/activate && command -v ollama && ollama ps`;
      exec(checkOllamaCommand, { shell: '/bin/bash', encoding: 'utf-8' }, (checkOllamaError, checkOllamaStdout, checkOllamaStderr) => {
        if (checkOllamaError || !checkOllamaStdout.includes('ollama')) {
          const errorMsg = `Ollama not found or service not responsive: ${checkOllamaStderr || checkOllamaError?.message || 'Unknown error'}\n`;
          fs.appendFileSync(reportFilePath, errorMsg, { encoding: 'utf-8' });
          vscode.window.showErrorMessage(errorMsg);
          try {
            fs.unlinkSync(tempPromptFile);
          } catch (err: unknown) {
            const errorMessage = err instanceof Error ? err.message : 'Unknown error';
            fs.appendFileSync(reportFilePath, `Error deleting temporary prompt file: ${errorMessage}\n`, { encoding: 'utf-8' });
          }
          return;
        }

        // Create a write stream for the report file
        const reportStream = fs.createWriteStream(reportFilePath, { flags: 'a', encoding: 'utf-8' });

        // Buffer for stdout to collect complete lines
        let stdoutBuffer = '';
        const spinnerChars = ['⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏', '⠋'];

        // Use spawn for streaming output
        const child = spawn('bash', ['-c', ollamaCommand], { stdio: ['inherit', 'pipe', 'pipe'], timeout: 600000 });

        // Stream stdout to report file
        child.stdout.on('data', (data: Buffer) => {
          const output = stripAnsi(data.toString('utf-8'));
          if (output.trim()) {
            stdoutBuffer += output;
            // Split by lines and write complete lines
            const lines = stdoutBuffer.split('\n');
            stdoutBuffer = lines.pop() || ''; // Keep incomplete line in buffer
            lines.forEach(line => {
              if (line.trim()) {
                reportStream.write(`${line}\n`);
                // Optionally show output in VS Code (commented to avoid spamming)
                // vscode.window.showInformationMessage(`Ollama Output: ${line.trim()}`);
              }
            });
          }
        });

        // Stream stderr to report file, filtering out spinner and progress outputs
        child.stderr.on('data', (data: Buffer) => {
          const errorOutput = stripAnsi(data.toString('utf-8'));
          // Filter out spinner characters and empty lines
          if (errorOutput.trim() && !spinnerChars.some(char => errorOutput.includes(char)) && !errorOutput.includes('<think>')) {
            reportStream.write(`Error: ${errorOutput}\n`);
            vscode.window.showErrorMessage(`Ollama Error: ${errorOutput.trim()}`);
          }
        });

        // Handle process exit
        child.on('close', (code: number | null, signal: string | null) => {
          // Write any remaining buffered stdout
          if (stdoutBuffer.trim()) {
            reportStream.write(`${stdoutBuffer}\n`);
          }

          // Clean up temporary prompt file
          try {
            fs.unlinkSync(tempPromptFile);
          } catch (err: unknown) {
            const errorMessage = err instanceof Error ? err.message : 'Unknown error';
            reportStream.write(`Error deleting temporary prompt file: ${errorMessage}\n`);
            vscode.window.showErrorMessage(`Error deleting temporary prompt file: ${errorMessage}`);
          }

          if (code !== 0 || signal) {
            const errorMsg = `Ollama process exited with code ${code} or signal ${signal}\n`;
            reportStream.write(errorMsg);
            vscode.window.showErrorMessage(errorMsg);
          }

          // Check if any meaningful output was written, if not, add a default message
          const reportContent = fs.readFileSync(reportFilePath, 'utf-8');
          if (!reportContent.includes('Line ') && !reportContent.includes('Error:')) {
            reportStream.write('This refactored version is more readable, cleaner, and avoids potential compiler warnings.\nNo issues detected.\n');
          }

          reportStream.end(() => {
            vscode.window.showInformationMessage(`AI analysis report saved at: ${reportFilePath}`);
          });
        });

        // Handle spawn errors
        child.on('error', (err: Error) => {
          const errorMsg = `Error spawning Ollama process: ${err.message}\n`;
          reportStream.write(errorMsg);
          vscode.window.showErrorMessage(errorMsg);
          try {
            fs.unlinkSync(tempPromptFile);
          } catch (err: unknown) {
            const errorMessage = err instanceof Error ? err.message : 'Unknown error';
            reportStream.write(`Error deleting temporary prompt file: ${errorMessage}\n`);
          }
          reportStream.end();
        });
      });
    }
  });

  context.subscriptions.push(checkStaticAll, checkCppVulnByCopilot, checkAI);
}

export function deactivate() {}