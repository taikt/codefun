import * as vscode from 'vscode';
import * as path from 'path';
import * as fs from 'fs';
import { exec } from 'child_process';
import { isCppFile } from '../helpers/fileUtils';
import { getReportDirectory } from '../extension';

export function registerCheckStaticAnalysisCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.checkStaticAnalysis', () => {
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
    // Lấy report directory từ user settings
    const reportDir = getReportDirectory();
    if (!fs.existsSync(reportDir)) {
      fs.mkdirSync(reportDir, { recursive: true });
    }
    const baseName = path.basename(filePath, path.extname(filePath));
    const mdReportPath = path.join(reportDir, `${baseName}.static-report.md`);
    const htmlReportPath = path.join(reportDir, `${baseName}.static-report.html`);
    let mdContent = `# C++ Static Vulnerability Report\n\n**Source File:** \`${filePath}\`\n\n`;
    let htmlContent = `<!DOCTYPE html><html>
<head><meta charset='utf-8'>
<title>Static Analysis Report</title>
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github.min.css">
<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
<script>hljs.highlightAll();</script>
<style>
* { margin: 0; padding: 0; box-sizing: border-box; }
body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; line-height: 1.6; color: #222; background: #fff; min-height: 100vh; font-size: 15px; }
.source-code-block { display: block; width: 100%; background: #fff; border-radius: 8px; border: 1px solid #e1e4e8; overflow: hidden; margin-top: 10px; }
.line-numbers-vertical { float: left; background: #f6f8fa; color: #586069; padding: 8px 12px 8px 16px; font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 14px; line-height: 1.5; user-select: none; text-align: right; border-right: 1px solid #e1e4e8; min-width: 50px; }
.line-numbers-vertical .line-number { display: block; height: 1.5em; }
.code-block { margin: 0 !important; padding: 8px 16px 8px 0 !important; border: none !important; background: #fff !important; font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 14px; line-height: 1.5; white-space: pre !important; overflow-x: auto; }
.code-block code { background: transparent !important; padding: 0 !important; display: block; }
.source-code-block:after { content: ""; display: table; clear: both; }
</style>
</head>
<body>
<h1>C++ Static Vulnerability Report</h1>
<p>
<b>Source File:</b>
<code>${filePath}</code>
</p>`;
    // Đọc code gốc và tạo code block với line number, highlight, giống LGEDV
    const codeContent = fs.readFileSync(filePath, 'utf-8');
    function escapeHtml(str: string) {
      return str.replace(/&/g, '&amp;')
                .replace(/</g, '&lt;')
                .replace(/>/g, '&gt;');
    }
    function codeWithLines(code: string) {
      const lines = code.split('\n');
      if (lines.length > 0 && lines[0].trim() === '') lines.shift();
      while (lines.length > 0 && lines[lines.length - 1].trim() === '') lines.pop();
      const lineNumbers = lines.map((_, i) => `<span class='line-number'>${i+1}</span>`).join('');
      const codeHtml = lines.map(escapeHtml).join('\n');
      // LGEDV style: line numbers and code block are siblings, not flex children
      return `<div class='source-code-block'>\n<div class='line-numbers-vertical'>${lineNumbers}</div>\n<pre class='code-block'><code class='language-cpp'>${codeHtml}</code></pre>\n</div>`;
    }
    htmlContent += `\n<div class='source-code-toggle' style='margin:10px 0 20px 0;'>\n<button id='show-src-btn' onclick='toggleSrcCode()' class='source-code-link' style='margin-bottom:8px;'>📄 Show original code</button>\n<div id='src-code-block' class='source-code-section' style='display:none;'>\n<h3>📄 Original Source Code</h3>\n<div class='code-content'>${codeWithLines(codeContent)}</div>\n</div>\n</div>\n<script>\nfunction toggleSrcCode() {\n  var block = document.getElementById('src-code-block');\n  var btn = document.getElementById('show-src-btn');\n  if (block.style.display === 'none') {\n    block.style.display = 'block';\n    btn.textContent = 'Hide original code';\n  } else {\n    block.style.display = 'none';\n    btn.textContent = '📄 Show original code';\n  }\n}\n</script>\n`;
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
        .filter((line) => !linesToExclude.includes(line.trim()))
        .join('\n');
      mdContent += `\n## === Clang-Tidy (LGEDV Rules) ===\n\n\`\`\`\n${stdout}\n${filteredStderr ? `Errors:\n${filteredStderr}` : ''}\n\`\`\`\n`;
      htmlContent += `<h2>Clang-Tidy (LGEDV Rules)</h2><pre>${stdout}${filteredStderr ? '\nErrors:\n' + filteredStderr : ''}</pre>`;
      // 2. cppcheck
      const cppcheckCmd = `cppcheck --enable=all --suppress=missingIncludeSystem "${filePath}"`;
      exec(cppcheckCmd, { encoding: 'utf-8' }, (cppError, cppStdout, cppStderr) => {
        const cppOutput = cppStdout + cppStderr;
        mdContent += `\n## === Cppcheck Analysis ===\n\n\`\`\`\n${cppOutput.trim() ? cppOutput : 'No issues detected.'}\n\`\`\`\n`;
        htmlContent += `<h2>Cppcheck Analysis</h2><pre>${cppOutput.trim() ? cppOutput : 'No issues detected.'}</pre>`;
        // 3. clang --analyze
        const clangCmd = `clang --analyze "${filePath}"`;
        exec(clangCmd, { encoding: 'utf-8' }, (clangError, clangStdout, clangStderr) => {
          const clangOutput = clangStdout + clangStderr;
          mdContent += `\n## === Clang Static Analyzer ===\n\n\`\`\`\n${clangOutput.trim() ? clangOutput : 'No issues detected.'}\n\`\`\`\n`;
          htmlContent += `<h2>Clang Static Analyzer</h2><pre>${clangOutput.trim() ? clangOutput : 'No issues detected.'}</pre></body></html>`;
          fs.writeFileSync(mdReportPath, mdContent, { encoding: 'utf-8' });
          fs.writeFileSync(htmlReportPath, htmlContent, { encoding: 'utf-8' });
          vscode.window.showInformationMessage(`Static analysis reports saved at: ${mdReportPath} and ${htmlReportPath}`);
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
  context.subscriptions.push(disposable);
}
