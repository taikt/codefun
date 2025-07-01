// Các hàm parse, extract, validate liên quan đến kết quả phân tích
import * as vscode from 'vscode';

export const parseViolationsFromResponse = (response: string): Array<{severity: string, ruleId?: string, lineNumber?: string, description?: string, originalCode?: string, fixedCode?: string, explanation?: string}> => {
  const violations = [];
  const issuePattern = /## 🚨 Issue (\d+): ([^\n]+)\n\*\*Rule Violated:\*\* ([^\n]+)\n\*\*Line[s]?:\*\* ([^\n]+)\n\*\*Severity:\*\* ([^\n]+)/g;
  let match;
  while ((match = issuePattern.exec(response)) !== null) {
    const [, issueNum, title, ruleInfo, lineInfo, severity] = match;
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
};

export const parseDetailedViolations = (response: string): any[] => {
  return parseViolationsFromResponse(response);
};

export const extractCompleteFixedCode = (response: string): string | null => {
  const fixedCodeMatch = response.match(/## 🔧 Complete Fixed Code\s*```cpp\s*([\s\S]*?)\s*```/);
  return fixedCodeMatch ? fixedCodeMatch[1].trim() : null;
};

export function validateRuleContent(rulesContent: string, ruleType: string, outputChannel: vscode.OutputChannel): {hasLGEDV: boolean, hasMISRA: boolean, hasCERT: boolean, hasRAPIDSCAN: boolean, hasCRITICAL: boolean} {
  const hasLGEDV = /LGEDV_CRCL_\d+/.test(rulesContent);
  const hasMISRA = /Rule \d+-\d+-\d+/.test(rulesContent);
  const hasCERT = /[A-Z]{3}\d{2}-CPP/.test(rulesContent);
  const hasRAPIDSCAN = /RS-\d+/.test(rulesContent);
  const hasCRITICAL = ruleType === 'CRITICAL' && rulesContent.length > 1000;
  outputChannel.appendLine(`📋 Rule validation: LGEDV=${hasLGEDV}, MISRA=${hasMISRA}, CERT=${hasCERT}, RapidScan=${hasRAPIDSCAN}, Critical=${hasCRITICAL}`);
  return { hasLGEDV, hasMISRA, hasCERT, hasRAPIDSCAN, hasCRITICAL };
}

export function extractRuleId(line: string): string {
  const ruleMatch = line.match(/(LGEDV_CRCL_\d+|Rule \d+-\d+-\d+|[A-Z]{3}\d{2}-CPP|RS-\d+)/);
  return ruleMatch ? ruleMatch[1] : 'Unknown Rule';
}

export function extractDescription(line: string): string {
  const descMatch = line.match(/Issue \d+: (.+?)(?:\*\*|$)/);
  return descMatch ? descMatch[1].trim() : 'Rule violation detected';
}

export function extractSeverity(line: string): string | null {
  const severityMatch = line.match(/\*\*Severity:\*\* (Critical|High|Medium|Low)/i);
  return severityMatch ? severityMatch[1] : null;
}

export function extractLineNumber(line: string): string {
  const lineMatch = line.match(/\*\*Line[s]?:\*\* (.+?)(?:\*\*|$)/);
  return lineMatch ? lineMatch[1].trim() : 'Unknown';
}
