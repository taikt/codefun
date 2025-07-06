"""
Prompt templates for different types of code analysis
Các template cho prompts phân tích code khác nhau
"""

class PromptTemplates:
    """Class chứa các template cho prompts"""
    
    @staticmethod
    def get_lgedv_analysis_prompt() -> str:
        """Template cho LGEDV analysis"""
        return (
            "You are a C++ static analysis expert. Analyze this code for violations of LGEDV rules for automotive code compliance.\n"
            "If the rule file is not existed, please calling fetch_lgedv_rule from MCP server.\n\n"
            "**IMPORTANT:**\n"
            "- Report all line numbers and function ranges according to the original file as provided below.\n"
            "**CODE TO ANALYZE:**\n"
            "```cpp\n[codeContent]\n```\n\n"
            "**RULES TO CHECK:**\n[ruleContent]\n\n"
            "**ANALYSIS REQUIREMENTS:**\n"
            "- Find ALL violations of the rules above\n"
            "- Focus specifically on LGEDV rule violations\n"
            "- Cite EXACT rule numbers (e.g., LGEDV_CRCL_0001, MISRA Rule 8-4-3, DCL50-CPP, RS-001)\n"
            "- Check every line thoroughly\n"
            "- Provide concrete fixes for each violation\n"
            "- Use the original file's line numbers in all reports\n\n"
            "**OUTPUT FORMAT:**\n"
            "For each violation found:\n\n"
            "## 🚨 Issue [#]: [Brief Description]\n"
            "**Rule Violated:** [EXACT_RULE_NUMBER] - [Rule Description]\n"
            "**Line:** [LINE_NUMBER]\n"
            "**Severity:** [Critical/High/Medium/Low]\n"
            "**Current Code:**\n"
            "```cpp\n[problematic code]\n```\n"
            "**Fixed Code:**\n"
            "```cpp\n[corrected code]\n```\n"
            "**Explanation:** [Why this violates the rule and how fix works]\n\n"
            "---\n\n"
            "## 🔧 Complete Fixed Code\n"
            "```cpp\n[entire corrected file with all fixes applied]\n```\n\n"
            "**Important:** If no LGEDV rule violations are found, clearly state \"No LGEDV rule violations detected in this code.\""
        )
    
    @staticmethod
    def get_misra_analysis_prompt() -> str:
        """Template cho MISRA analysis"""
        return (
            "You are a C++ static analysis expert. Analyze this code for violations of MISRA C++ 2008 rules for safety-critical software.\n"
            "If the rule file is not existed, please calling fetch_misra_rule from MCP server.\n\n"
            "**IMPORTANT:**\n"
            "- Report all line numbers and function ranges according to the original file as provided below.\n"
            "```cpp\n[codeContent]\n```\n\n"
            "**RULES TO CHECK:**\n[ruleContent]\n\n"
            "**ANALYSIS REQUIREMENTS:**\n"                       
            "- Find ALL violations of the rules above\n"
            "- Focus specifically on MISRA rule violations\n"
            "- Cite EXACT rule numbers (e.g., MISRA Rule 8-4-3, LGEDV_CRCL_0001, DCL50-CPP, RS-001)\n"
            "- Check every line thoroughly\n"
            "- Provide concrete fixes for each violation\n"
            "- Use the original file's line numbers in all reports\n\n"
            "**OUTPUT FORMAT:**\n"
            "For each violation found:\n\n"
            "## 🚨 Issue [#]: [Brief Description]\n"
            "**Rule Violated:** [EXACT_RULE_NUMBER] - [Rule Description]\n"
            "**Line:** [LINE_NUMBER]\n"
            "**Severity:** [Critical/High/Medium/Low]\n"
            "**Current Code:**\n"
            "```cpp\n[problematic code]\n```\n"
            "**Fixed Code:**\n"
            "```cpp\n[corrected code]\n```\n"
            "**Explanation:** [Why this violates the rule and how fix works]\n\n"
            "---\n\n"
            "## 🔧 Complete Fixed Code\n"
            "```cpp\n[entire corrected file with all fixes applied]\n```\n\n"
            "**Important:** If no MISRA rule violations are found, clearly state \"No MISRA rule violations detected in this code.\""
        )
    
    @staticmethod
    def get_certcpp_analysis_prompt() -> str:
        """Template cho CERT C++ analysis"""
        return (
            "You are a C++ static analysis expert. Analyze this code for violations of CERT C++ Secure Coding Standard rules.\n"
            "If the rule file is not existed, please calling fetch_certcpp_rule from MCP server.\n\n"
            "**IMPORTANT:**\n"
            "- Report all line numbers and function ranges according to the original file as provided below.\n"
            "```cpp\n[codeContent]\n```\n\n"
            "**RULES TO CHECK:**\n[ruleContent]\n\n"
            "**ANALYSIS REQUIREMENTS:**\n"                       
            "- Find ALL violations of the rules above\n"
            "- Focus specifically on CERT rule violations\n"
            "- Cite EXACT rule numbers (e.g., DCL50-CPP, MISRA Rule 8-4-3, LGEDV_CRCL_0001, RS-001)\n"
            "- Check every line thoroughly\n"
            "- Provide concrete fixes for each violation\n"
            "- Use the original file's line numbers in all reports\n\n"
            "**OUTPUT FORMAT:**\n"
            "For each violation found:\n\n"
            "## 🚨 Issue [#]: [Brief Description]\n"
            "**Rule Violated:** [EXACT_RULE_NUMBER] - [Rule Description]\n"
            "**Line:** [LINE_NUMBER]\n"
            "**Severity:** [Critical/High/Medium/Low]\n"
            "**Current Code:**\n"
            "```cpp\n[problematic code]\n```\n"
            "**Fixed Code:**\n"
            "```cpp\n[corrected code]\n```\n"
            "**Explanation:** [Why this violates the rule and how fix works]\n\n"
            "---\n\n"
            "## 🔧 Complete Fixed Code\n"
            "```cpp\n[entire corrected file with all fixes applied]\n```\n\n"
            "**Important:** If no CERT rule violations are found, clearly state \"No CERT rule violations detected in this code.\""
        )
    
    @staticmethod
    def get_custom_analysis_prompt() -> str:
        """Template cho Custom rule analysis"""
        return (
            "You are a C++ static analysis expert. Analyze this code for violations of the following custom rules.\n"
            "If the rule file is not existed, please calling fetch_custom_rule from MCP server.\n\n"
            "**IMPORTANT:**\n"
            "- Report all line numbers and function ranges according to the original file as provided below.\n"
            "**CODE TO ANALYZE:**\n"
            "```cpp\n[codeContent]\n```\n\n"
            "**RULES TO CHECK:**\n[ruleContent]\n\n"
            "**ANALYSIS REQUIREMENTS:**\n"                        
            "- Find ALL violations of the rules above\n"
            "- Focus specifically on custom rule violations\n"
            "- Cite EXACT rule numbers (e.g., CUSTOM-001, MISRA Rule 8-4-3, LGEDV_CRCL_0001, RS-001)\n"
            "- Check every line thoroughly\n"
            "- Provide concrete fixes for each violation\n"
            "- Use the original file's line numbers in all reports\n\n"
            "**OUTPUT FORMAT:**\n"
            "For each violation found:\n\n"
            "## 🚨 Issue [#]: [Brief Description]\n"
            "**Rule Violated:** [EXACT_RULE_NUMBER] - [Rule Description]\n"
            "**Line:** [LINE_NUMBER]\n"
            "**Severity:** [Critical/High/Medium/Low]\n"
            "**Current Code:**\n"
            "```cpp\n[problematic code]\n```\n"
            "**Fixed Code:**\n"
            "```cpp\n[corrected code]\n```\n"
            "**Explanation:** [Why this violates the rule and how fix works]\n\n"
            "---\n\n"
            "## 🔧 Complete Fixed Code\n"
            "```cpp\n[entire corrected file with all fixes applied]\n```\n\n"
            "**Important:** If no custom rule violations are found, clearly state \"No custom rule violations detected in this code.\""
        )
    
    @staticmethod
    def get_race_condition_analysis_prompt(context_summary: str) -> str:
        """Template cho race condition analysis"""
        return f"""You are an expert C++ concurrency analyst. Analyze the provided codebase context for **RACE CONDITIONS**.

{context_summary}

**ANALYSIS REQUIREMENTS:**
1. **Deep Analysis**: Look beyond basic regex detection - analyze data flow, synchronization patterns, and concurrent access patterns
2. **Race Condition Types**: Identify data races, deadlocks, missing synchronization, atomic operation issues
3. **Cross-file Analysis**: Focus on resources accessed across multiple files, especially those with threading
4. **Severity Assessment**: Rank issues by severity (Critical/High/Medium/Low)
5. **Fix Recommendations**: Provide specific synchronization strategies (mutex, atomic, locks, etc.)

**OUTPUT FORMAT:**
For each race condition found:

## 🚨 **RACE CONDITION #[number]**: [Brief Description]
**Type:** [data_race|deadlock|missing_sync|atomic_violation]
**Severity:** [Critical|High|Medium|Low]
**Files Involved:** [list of files]
**Resources:** [shared resources involved]

**Problem Description:**
[Detailed explanation of the race condition]

**Code Locations:**
```cpp
// File: filename.cpp, Lines: X-Y
[relevant code snippet]
```

**Fix Recommendation:**
```cpp
// Suggested fix with proper synchronization
[corrected code with mutex/atomic/etc]
```

---

## 📊 **SUMMARY**
- **Total Race Conditions Found:** [number]
- **Critical Issues:** [number]
- **Recommended Priority:** [order of fixes]
- **Overall Assessment:** [general codebase thread safety assessment]

Please call the `detect_race_conditions` tool first to get additional details if needed, then provide your expert analysis."""
