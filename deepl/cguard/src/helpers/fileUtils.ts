// Các hàm thao tác file, kiểm tra tồn tại, đọc rule file, optimize rule
import * as fs from 'fs';
import * as path from 'path';
import * as vscode from 'vscode';

export async function fileExists(filePath: string): Promise<boolean> {
  try {
    await fs.promises.access(filePath);
    return true;
  } catch {
    return false;
  }
}

export async function loadRuleFile(fileName: string, ruleSetDir: string, outputChannel: vscode.OutputChannel): Promise<string | null> {
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

export function optimizeRulesForPrompt(rulesContent: string, ruleType: string, maxSize: number): string {
  if (ruleType === 'CRITICAL') {
    const criticalMatch = rulesContent.match(/# Critical Rules\n([\s\S]*?)(?=\n# |$)/);
    if (criticalMatch) {
      return `# Critical Rules\n${criticalMatch[1].substring(0, maxSize)}\n\n`;
    }
  }
  return rulesContent.substring(0, maxSize);
}

export function isCppFile(filePath: string): boolean {
  const ext = filePath.toLowerCase();
  return ext.endsWith('.cpp') || ext.endsWith('.cc') || ext.endsWith('.cxx') || ext.endsWith('.h') || ext.endsWith('.hpp') || ext.endsWith('.hxx');
}

export function fileExistsSync(filePath: string): boolean {
    try {
        return fs.existsSync(filePath);
    } catch {
        return false;
    }
}

export function loadRuleFileSync(ruleFilePath: string): string | null {
    try {
        return fs.readFileSync(ruleFilePath, 'utf8');
    } catch {
        return null;
    }
}

export function createLineByLineDiff(original: string, modified: string): string {
    const jsdiff = require('diff');
    const diff = jsdiff.diffLines(original, modified);
    let result = '';
    diff.forEach((part: any) => {
        const prefix = part.added ? '+' : part.removed ? '-' : ' ';
        result += prefix + part.value;
    });
    return result;
}

export function createCodeWithLines(code: string): string {
  // Escape HTML special characters
  function escapeHtml(str: string): string {
    return str.replace(/&/g, '&amp;')
              .replace(/</g, '&lt;')
              .replace(/>/g, '&gt;');
  }
  const lines = code.split('\n');
  const lineNumbers = lines.map((_, idx) => `<span class="line-number">${idx + 1}</span>`).join('\n');
  // Wrap each line in <span style='white-space:pre'> to preserve indentation
  const codeHtml = lines.map(l => `<span style="white-space:pre">${escapeHtml(l)}</span>`).join('\n');
  return `
    <div class="code-with-lines">
      <div class="line-numbers">${lineNumbers}</div>
      <pre><code class="language-cpp">${codeHtml}</code></pre>
    </div>
  `;
}
