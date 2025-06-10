"use strict";
var __createBinding = (this && this.__createBinding) || (Object.create ? (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    var desc = Object.getOwnPropertyDescriptor(m, k);
    if (!desc || ("get" in desc ? !m.__esModule : desc.writable || desc.configurable)) {
      desc = { enumerable: true, get: function() { return m[k]; } };
    }
    Object.defineProperty(o, k2, desc);
}) : (function(o, m, k, k2) {
    if (k2 === undefined) k2 = k;
    o[k2] = m[k];
}));
var __setModuleDefault = (this && this.__setModuleDefault) || (Object.create ? (function(o, v) {
    Object.defineProperty(o, "default", { enumerable: true, value: v });
}) : function(o, v) {
    o["default"] = v;
});
var __importStar = (this && this.__importStar) || (function () {
    var ownKeys = function(o) {
        ownKeys = Object.getOwnPropertyNames || function (o) {
            var ar = [];
            for (var k in o) if (Object.prototype.hasOwnProperty.call(o, k)) ar[ar.length] = k;
            return ar;
        };
        return ownKeys(o);
    };
    return function (mod) {
        if (mod && mod.__esModule) return mod;
        var result = {};
        if (mod != null) for (var k = ownKeys(mod), i = 0; i < k.length; i++) if (k[i] !== "default") __createBinding(result, mod, k[i]);
        __setModuleDefault(result, mod);
        return result;
    };
})();
Object.defineProperty(exports, "__esModule", { value: true });
exports.activate = activate;
exports.deactivate = deactivate;
const vscode = __importStar(require("vscode"));
const child_process_1 = require("child_process");
const fs = __importStar(require("fs"));
const path = __importStar(require("path"));
// Function to strip ANSI escape codes
const stripAnsi = (text) => {
    return text.replace(/\x1B\[[0-?]*[ -/]*[@-~]/g, '');
};
// Check if a file is a C++ file based on its extension
const isCppFile = (filename) => {
    const cppExtensions = ['.cpp', '.h', '.hpp', '.cxx', '.cc'];
    return cppExtensions.includes(path.extname(filename).toLowerCase());
};
function activate(context) {
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
        (0, child_process_1.exec)(clangTidyCmd, (error, stdout, stderr) => {
            const linesToExclude = [
                '99 warnings generated.',
                'Suppressed 98 warnings (98 in non-user code).',
                'Use -header-filter=.* to display errors from all non-system headers. Use -system-headers to display errors from system headers as well.'
            ];
            const filteredStderr = stderr
                .split('\n')
                .filter((line) => !linesToExclude.includes(line.trim()))
                .join('\n');
            const clangTidyContent = `=== Clang-Tidy (LGEDV Rules) ===\n${stdout}\n${filteredStderr ? `Errors:\n${filteredStderr}` : ''}\n`;
            fs.appendFileSync(reportFilePath, clangTidyContent, { encoding: 'utf-8' });
            // 2. cppcheck
            const cppcheckCmd = `cppcheck --enable=all --suppress=missingIncludeSystem "${filePath}"`;
            (0, child_process_1.exec)(cppcheckCmd, { encoding: 'utf-8' }, (cppError, cppStdout, cppStderr) => {
                const cppOutput = cppStdout + cppStderr;
                const cppContent = `\n=== Cppcheck Analysis ===\n${cppOutput.trim() ? cppOutput : 'No issues detected.'}\n`;
                fs.appendFileSync(reportFilePath, cppContent, { encoding: 'utf-8' });
                // 3. clang --analyze
                const clangCmd = `clang --analyze "${filePath}"`;
                (0, child_process_1.exec)(clangCmd, { encoding: 'utf-8' }, (clangError, clangStdout, clangStderr) => {
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
        const misraGuideFileName = 'Misracpp2008Guidelines.md';
        const guideFileName = 'LGEDVRuleGuide.md';
        const prompt = `Check for C++ rule violation based on the guidelines in ${misraGuideFileName} and ${guideFileName}.\n`
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
            vscode.window.showInformationMessage('Prompt has been copied to clipboard. Please open Copilot Chat and paste the prompt to check the rules.');
        }
    });
    // Check C++ vulnerability by local AI (Ollama via Python script)
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
        console.log('AI report file will be saved at:', reportFilePath);
        // Check if Ollama is running (by checking if port 11434 is open)
        const net = require('net');
        function checkOllamaRunning(callback) {
            const client = new net.Socket();
            client.setTimeout(1000);
            client.once('error', () => { client.destroy(); callback(false); });
            client.once('timeout', () => { client.destroy(); callback(false); });
            client.connect(11434, '127.0.0.1', () => {
                client.end();
                callback(true);
            });
        }
        checkOllamaRunning((isRunning) => {
            if (!isRunning) {
                const msg = 'Ollama chưa chạy, cần chạy Ollama trước khi phân tích AI.';
                fs.appendFileSync(reportFilePath, msg + '\n', { encoding: 'utf-8' });
                vscode.window.showErrorMessage(msg);
                return;
            }
            // Nếu Ollama đã chạy, gọi script Python để phân tích
            proceedWithOllama();
        });
        function proceedWithOllama() {
            // Gọi script Python llama.py với các tham số cần thiết
            const pythonScript = '/home/worker/src/dl/llama.py';
            // --ollama-only là tuỳ chọn, bạn có thể thêm logic trong llama.py để chỉ chạy Ollama nếu có tham số này
            const pythonCmd = `python3 "${pythonScript}" --file "${filePath}" --output "${reportFilePath}" --ollama-only`;
            vscode.window.showInformationMessage(`Đang phân tích ${filePath} với Ollama (qwen3:8b) qua Python...`);
            (0, child_process_1.exec)(pythonCmd, { encoding: 'utf-8' }, (error, stdout, stderr) => {
                if (error) {
                    const errorMsg = `Lỗi khi gọi Ollama qua Python: ${stderr || error.message || 'Unknown error'}\n`;
                    fs.appendFileSync(reportFilePath, errorMsg, { encoding: 'utf-8' });
                    vscode.window.showErrorMessage(errorMsg);
                    return;
                }
                vscode.window.showInformationMessage('Ollama analysis completed (qua Python).');
            });
        }
    });
    context.subscriptions.push(checkStaticAll, checkCppVulnByCopilot, checkAI);
}
function deactivate() { }
//# sourceMappingURL=extension%20copy.js.map