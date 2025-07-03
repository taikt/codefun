import * as path from 'path';

// --- Strict copy of helpers from extension_bk.ts ---

// Helper: Chuẩn hóa lề trái cho code block (loại bỏ số space chung nhỏ nhất ở đầu mỗi dòng, trừ dòng trống)
function normalizeIndentation(code: string): string {
  const lines = code.split('\n');
  // Bỏ qua dòng trống đầu/cuối
  while (lines.length > 0 && lines[0].trim() === '') lines.shift();
  while (lines.length > 0 && lines[lines.length - 1].trim() === '') lines.pop();
  // Tìm số space chung nhỏ nhất ở đầu mỗi dòng (bỏ qua dòng trống)
  const indents = lines
    .filter(line => line.trim() !== '')
    .map(line => line.match(/^\s*/)?.[0].length ?? 0);
  const minIndent = indents.length > 0 ? Math.min(...indents) : 0;
  // Loại bỏ minIndent space ở đầu mỗi dòng (chỉ nếu > 0)
  return minIndent > 0
    ? lines.map(line => line.startsWith(' '.repeat(minIndent)) ? line.slice(minIndent) : line).join('\n')
    : lines.join('\n');
}

// Helper: Parse violations from response
function parseViolationsFromResponse(response: string): Array<{severity: string, ruleId?: string, lineNumber?: string, description?: string, originalCode?: string, fixedCode?: string, explanation?: string}> {
  const violations = [];
  const issuePattern = /## 🚨 Issue (\d+): ([^\n]+)\n\*\*Rule Violated:\*\* ([^\n]+)\n\*\*Line[s]?:\*\* ([^\n]+)\n\*\*Severity:\*\* ([^\n]+)/g;
  let match;
  while ((match = issuePattern.exec(response)) !== null) {
    const [, issueNum, title, ruleInfo, lineInfo, severity] = match;
    const issueText = response.substring(match.index, response.indexOf('---', match.index + match[0].length));
    const currentCodeMatch = issueText.match(/\*\*Current Code:\*\*\s*```cpp\s*([\s\S]*?)\s*```/);
    const fixedCodeMatch = issueText.match(/\*\*Fixed Code:\*\*\s*```cpp\s*([\s\S]*?)\s*```/);
    const explanationMatch = issueText.match(/\*\*Explanation:\*\*\s*([^\n]+)/);
    // Loại bỏ dòng trắng đầu/cuối, KHÔNG dùng .trim() để giữ nguyên indentation
    function stripBlankLinesPreserveIndent(code: string | undefined): string {
      if (!code) return '';
      const lines = code.split('\n');
      while (lines.length > 0 && lines[0].trim() === '') lines.shift();
      while (lines.length > 0 && lines[lines.length - 1].trim() === '') lines.pop();
      return lines.join('\n');
    }
    violations.push({
      severity: severity.trim(),
      ruleId: ruleInfo.split(' - ')[0]?.trim() || `Issue ${issueNum}`,
      lineNumber: lineInfo.trim(),
      description: title.trim(),
      originalCode: currentCodeMatch ? stripBlankLinesPreserveIndent(currentCodeMatch[1]) : '',
      fixedCode: fixedCodeMatch ? stripBlankLinesPreserveIndent(fixedCodeMatch[1]) : '',
      explanation: explanationMatch ? explanationMatch[1].trim() : ''
    });
  }
  return violations;
}

// Helper: Extract complete fixed code
function extractCompleteFixedCode(response: string): string | null {
  const fixedCodeMatch = response.match(/## 🔧 Complete Fixed Code\s*```cpp\s*([\s\S]*?)\s*```/);
  return fixedCodeMatch ? fixedCodeMatch[1].trim() : null;
}

// Helper: Count violations by severity
export function countViolationsBySeverity(violations: Array<{severity: string}>): {critical: number, high: number, medium: number, low: number} {
  return {
    critical: violations.filter(v => v.severity === 'Critical').length,
    high: violations.filter(v => v.severity === 'High').length,
    medium: violations.filter(v => v.severity === 'Medium').length,
    low: violations.filter(v => v.severity === 'Low').length
  };
}

// Helper: Create code block with line numbers
function createCodeWithLines(code: string, language: string = 'cpp'): string {
  // KHÔNG normalizeIndentation nữa, giữ nguyên code gốc
  const lines = code.split('\n');
  if (lines.length > 0 && lines[0].trim() === '') lines.shift();
  while (lines.length > 0 && lines[lines.length - 1].trim() === '') lines.pop();
  const lineNumbers = lines.map((_, index) => `<span class=\"line-number\">${index + 1}</span>`).join('\n');
  // Only escape HTML, do not replace leading spaces with &nbsp;; let <pre> handle indentation
  const escapedCode = lines.map(line =>
    line
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
  ).join('\n');
  return `<div class=\"code-with-lines\"><div class=\"line-numbers\">${lineNumbers}</div><pre style=\"white-space: pre;\"><code class=\"language-${language}\">${escapedCode}</code></pre></div>`;
}

// --- Strict copy of generateMarkdownReport from extension_bk.ts ---
export async function generateMarkdownReport(
  baseFileName: string,
  filePath: string, 
  ruleType: string, 
  response: string, 
  codeContent: string, 
  modelName: string, 
  timestamp: string
): Promise<string> {
  const violations = parseViolationsFromResponse(response);
  const severityCounts = countViolationsBySeverity(violations);
  return `# 🛡️ ${ruleType} Analysis Report\n\n**File:** \`${path.basename(filePath)}\`  \n**Path:** \`${filePath}\`  \n**Analyzed:** ${new Date(timestamp).toLocaleString()}  \n**Rule Type:** ${ruleType} Coding Standards  \n**Model:** ${modelName}  \n**Lines of Code:** ${codeContent.split('\n').length}  \n**Code Size:** ${codeContent.length} characters  \n**Total Issues:** ${violations.length} violations found\n\n---\n\n## 📊 Summary Dashboard\n\n| Severity | Count | Description |\n|----------|-------|-------------|\n| 🔴 Critical | ${severityCounts.critical} | Must fix immediately |\n| 🟠 High | ${severityCounts.high} | Fix before release |\n| 🟡 Medium | ${severityCounts.medium} | Improve code quality |\n| 🔵 Low | ${severityCounts.low} | Best practice recommendations |\n\n---\n\n## 📋 Analysis Results\n\n${response}\n\n---\n\n## 📄 Source Code Analyzed\n\n\`\`\`cpp\n${codeContent}\n\`\`\`\n\n---\n\n*Generated by LGEDV CodeGuard v1.0.0 - ${ruleType} Analysis*\n`;
}

// --- Strict copy of generateInteractiveHTMLReport from extension_bk.ts ---
export async function generateInteractiveHTMLReport(
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
<html lang=\"en\">
<head>
    <meta charset=\"UTF-8\">
    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">
    <title>${ruleType} Code Analysis Report - ${path.basename(filePath)}</title>
    <link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/vs2015.min.css\">
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; line-height: 1.6; color: #333; background: #f5eee6; min-height: 100vh; }
        .container { max-width: 100%; margin: 0; padding: 20px; width: 100vw; box-sizing: border-box; }
        .header { background: white; border-radius: 15px; padding: 30px; margin-bottom: 30px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
        .header h1 { color: #2c3e50; font-size: 2.5em; margin-bottom: 10px; }
        .meta { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin-top: 20px; }
        .meta-item { background: rgba(255,255,255,0.1); padding: 10px 15px; border-radius: 8px; backdrop-filter: blur(10px); word-break: break-all; }
        .summary { background: white; border-radius: 15px; padding: 30px; margin-bottom: 30px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
        .summary h2 { color: #2c3e50; margin-bottom: 20px; }
        .severity-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin-top: 20px; }
        .severity-card { background: #f8f9fa; padding: 20px; border-radius: 10px; text-align: center; border-left: 4px solid #ddd; }
        .severity-card.critical { border-left-color: #dc3545; }
        .severity-card.high { border-left-color: #fd7e14; }
        .severity-card.medium { border-left-color: #ffc107; }
        .severity-card.low { border-left-color: #28a745; }
        .source-code-section { background: #f8f9fa; padding: 20px; border-radius: 10px; margin-top: 20px; border: 2px solid #e9ecef; }
        .source-code-section h3 { color: #2c3e50; margin-bottom: 15px; }
        .source-code-info { color: #6c757d; font-size: 0.9em; margin-bottom: 15px; }
        .source-code-toggle { margin-top: 20px; text-align: center; }
        .source-code-link { display: inline-block; padding: 12px 20px; background: #007acc; color: white; text-decoration: none; border-radius: 8px; font-weight: 500; transition: background-color 0.3s ease; }
        .source-code-link:hover { background: #005a9e; text-decoration: none; }
        .source-code-link:focus { outline: 2px solid #007acc; outline-offset: 2px; }
        .code-with-lines { display: flex; background: #ffffff; border-radius: 8px; overflow: hidden; width: 100%; border: 1px solid #e1e4e8; }
        .code-with-lines::-webkit-scrollbar { width: 8px; height: 8px; }
        .code-with-lines::-webkit-scrollbar-track { background: #1e1e1e; border-radius: 4px; }
        .code-with-lines::-webkit-scrollbar-thumb { background: #888; border-radius: 4px; }
        .code-with-lines::-webkit-scrollbar-thumb:hover { background: #aaa; }
        .code-with-lines .line-numbers { background: #f6f8fa !important; color: #586069; padding: 16px 12px 16px 16px; font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 14px; line-height: 1.5; user-select: none; text-align: right; border-right: 1px solid #e1e4e8; min-width: 50px; }
        .code-with-lines .line-numbers .line-number { display: block; height: 1.5em; }
        .code-with-lines pre { margin: 0 !important; border-radius: 0 !important; flex: 1; background: #1e1e1e !important; border: none !important; }
        .code-with-lines pre code { background: transparent !important; padding: 0 !important; font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 14px; line-height: 1.5; white-space: pre !important; }
        .source-code-with-lines { display: flex; background: #ffffff !important; border-radius: 8px; overflow: hidden; width: 100%; border: 1px solid #e1e4e8; }
        .source-code-with-lines::-webkit-scrollbar { width: 8px; height: 8px; }
        .source-code-with-lines::-webkit-scrollbar-track { background: #ffffff; border-radius: 4px; }
        .source-code-with-lines::-webkit-scrollbar-thumb { background: #888; border-radius: 4px; }
        .source-code-with-lines::-webkit-scrollbar-thumb:hover { background: #aaa; }
        .source-code-with-lines .line-numbers { background: #f6f8fa !important; color: #586069; padding: 16px 12px 16px 16px; font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 14px; line-height: 1.5; user-select: none; text-align: right; border-right: 1px solid #e1e4e8; min-width: 50px; }
        .source-code-with-lines .line-numbers .line-number { display: block; height: 1.5em; }
        .source-code-with-lines pre { margin: 0 !important; padding: 0 !important; border-radius: 0 !important; flex: 1; background: #ffffff !important; border: none !important; overflow: visible; }
        .source-code-with-lines pre code { background: transparent !important; padding: 8px 16px 8px 0 !important; font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 14px; line-height: 1.5; display: block; white-space: pre !important; }
        .content { background: white; border-radius: 15px; padding: 30px; margin-bottom: 30px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
        .violation { border: 1px solid #e9ecef; border-radius: 10px; margin-bottom: 20px; overflow: hidden; background: white; }
        .violation-header { background: #f8f9fa; padding: 20px; border-bottom: 1px solid #e9ecef; }
        .violation-header h3 { margin-bottom: 10px; color: #2c3e50; }
        .code-comparison { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; padding: 20px; }
        .code-section h4 { color: #2c3e50; margin-bottom: 15px; padding: 10px; background: #ecf0f1; border-radius: 5px; }
        .code-content { position: relative; }
        .code-content pre { margin: 0 !important; border-radius: 8px !important; background: #ffffff !important; width: 100%; border: 1px solid #e1e4e8; }
        .code-content pre::-webkit-scrollbar { width: 8px; height: 8px; }
        .code-content pre::-webkit-scrollbar-track { background: #ffffff; border-radius: 4px; }
        .code-content pre::-webkit-scrollbar-thumb { background: #888; border-radius: 4px; }
        .code-content pre::-webkit-scrollbar-thumb:hover { background: #aaa; }
        .hljs { background: #ffffff !important; color: #24292e !important; font-family: 'Consolas', 'Monaco', 'Courier New', monospace !important; font-size: 14px !important; line-height: 1.5 !important; padding: 0 !important; margin: 0 !important; border: none !important; border-radius: 0 !important; }
        .code-with-lines .hljs { background: #ffffff !important; border-radius: 0 !important; padding: 0 !important; margin: 0 !important; }
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
        .line-numbers { background: #f6f8fa !important; color: #586069; padding: 16px 12px 16px 16px; font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 14px; line-height: 1.5; user-select: none; text-align: right; border-right: 1px solid #e1e4e8 !important; min-width: 50px; }
        .code-with-lines .line-numbers { background: #f6f8fa !important; color: #586069; padding: 16px 12px 16px 16px; font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 14px; line-height: 1.5; user-select: none; text-align: right; border-right: 1px solid #e1e4e8 !important; min-width: 50px; }
        .code-with-lines pre { margin: 0 !important; border-radius: 0 !important; flex: 1; background: #ffffff !important; padding: 0 !important; overflow: visible; }
        .code-with-lines pre code { background: transparent !important; padding: 8px 16px 8px 0 !important; font-family: 'Fira Code', 'Consolas', 'Monaco', 'Courier New', monospace; font-size: 14px; line-height: 1.5; display: block; white-space: pre !important; }
        /* Code blocks - no collapse functionality */
        .source-code-section, .fixed-code-section { margin-top: 30px; padding: 20px; border-radius: 10px; border: 2px solid; }
        .fixed-code-section { background: linear-gradient(135deg, #e8f5e8, #c8e6c9); border-color: #4caf50; }
        .source-code-section { background: linear-gradient(135deg, #f3e5f5, #e1bee7); border-color: #9c27b0; }
        .explanation-box { background: linear-gradient(135deg, #e3f2fd, #bbdefb); border: 1px solid #2196f3; border-radius: 8px; padding: 15px; margin-top: 15px; }
        /* Gerrit-style side-by-side diff styles */
        .complete-code-comparison { margin-top: 30px; padding: 20px; border-radius: 10px; background: linear-gradient(135deg, #f8f9fa, #e9ecef); border: 2px solid #6c757d; }
        .gerrit-style-diff { background: white; border: 1px solid #d0d7de; border-radius: 8px; overflow: hidden; margin-top: 15px; font-family: 'Consolas', 'Monaco', 'Courier New', monospace; }
        .diff-header { display: grid; grid-template-columns: 1fr 1fr; background: #f6f8fa; border-bottom: 1px solid #d0d7de; }
        .diff-file-header { padding: 12px 16px; font-weight: 600; font-size: 14px; border-right: 1px solid #d0d7de; text-align: center; }
        .diff-file-header:last-child { border-right: none; }
        .diff-file-header.original { background: #fff1f0; color: #d73a49; border-right: 1px solid #d0d7de; }
        .diff-file-header.fixed { background: #f0fff4; color: #28a745; }
        .diff-content { display: grid; grid-template-columns: 1fr 1fr; align-items: start; overflow: hidden; }
        .diff-side { position: relative; overflow: auto; border-right: 1px solid #d0d7de; scrollbar-width: thin; scrollbar-color: #888 #1e1e1e; }
        .diff-side::-webkit-scrollbar { width: 8px; height: 8px; }
        .diff-side::-webkit-scrollbar-track { background: #1e1e1e; border-radius: 4px; }
        .diff-side::-webkit-scrollbar-thumb { background: #888; border-radius: 4px; }
        .diff-side::-webkit-scrollbar-thumb:hover { background: #aaa; }
        .diff-side:last-child { border-right: none; }
        .diff-side.original { background: #ffeef0; }
        .diff-side.fixed { background: #f0fff4; }
        .diff-side .code-content { height: 100%; position: relative; overflow: auto; }
        .diff-side .code-content .code-with-lines { height: auto; background: #ffffff !important; border-radius: 0 !important; max-height: none; overflow: visible; }
        .diff-side .code-content .line-numbers { background: #f6f8fa !important; color: #586069 !important; border-right: 1px solid #e1e4e8 !important; position: sticky; left: 0; z-index: 1; }
        .diff-side .code-content pre { margin: 0 !important; border-radius: 0 !important; height: auto; max-height: none !important; white-space: pre !important; word-wrap: break-word; font-size: 14px; line-height: 1.4; padding: 16px; overflow: visible; }
        .diff-side .hljs { background: #ffffff !important; color: #24292e !important; font-family: 'Consolas', 'Monaco', 'Courier New', monospace !important; font-size: 14px !important; line-height: 1.45 !important; padding: 16px !important; border-radius: 0 !important; }
        .diff-line { display: block; min-height: 1.45em; padding: 0 8px; border-left: 3px solid transparent; }
        .diff-line.removed { background-color: #ffeef0; border-left-color: #f85149; }
        .diff-line.added { background-color: #f0fff4; border-left-color: #3fb950; }
        .diff-line.context { background-color: transparent; }
        .diff-line-number { display: inline-block; width: 40px; text-align: right; color: #656d76; font-size: 12px; margin-right: 8px; user-select: none; }
        @media (max-width: 768px) { .code-comparison { grid-template-columns: 1fr; } .meta { grid-template-columns: 1fr; } .severity-grid { grid-template-columns: 1fr; } .diff-header { grid-template-columns: 1fr; } .diff-content { grid-template-columns: 1fr; } .diff-side { border-right: none; border-bottom: 1px solid #d0d7de; } .diff-side:last-child { border-bottom: none; } .diff-file-header { border-right: none; border-bottom: 1px solid #d0d7de; } .diff-file-header:last-child { border-bottom: none; } }
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
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>
    <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/cpp.min.js"></script>
    <script>
        document.addEventListener('DOMContentLoaded', function() {
            hljs.highlightAll();
            console.log('✅ Highlight.js initialized - syntax highlighting applied');
        });
        function toggleSourceCode() {
            const sections = document.querySelectorAll('#source-code-section, #source-code-section-2');
            const links = document.querySelectorAll('.source-code-link');
            sections.forEach((section, index) => {
                if (section) {
                    const isHidden = section.style.display === 'none';
                    section.style.display = isHidden ? 'block' : 'none';
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
