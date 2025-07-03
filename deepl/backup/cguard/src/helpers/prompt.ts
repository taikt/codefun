// Hàm tạo prompt cho Copilot
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
- When returning any code block (Current Code, Fixed Code, Complete Fixed Code), you MUST preserve all leading whitespace, indentation, and blank lines exactly as in the original or as intended in your fix. Do NOT left-align, trim, or otherwise alter the indentation of any code lines.
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
