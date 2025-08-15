import re
import sys

def extract_violations(md_text):
    # Hỗ trợ cả tiếng Anh và tiếng Việt
    issue_patterns = [
        r'## 🚨 Issue\s*\d*:? ?([^\n]*)',  # English, có thể có số
        r'## 🚨 Vấn đề\s*\d*:? ?([^\n]*)'  # Vietnamese, có thể có số
    ]
    rule_patterns = [
        r'\*\*Rule Violated:\*\*\s*([^\n]*)',  # English
        r'\*\*Rule vi phạm:\*\*\s*([^\n]*)'  # Vietnamese
    ]
    location_patterns = [
        r'\*\*Location:\*\*\s*([^\n]*)',  # English
        r'\*\*Vị trí:\*\*\s*([^\n]*)'  # Vietnamese
    ]
    severity_patterns = [
        r'\*\*Severity:\*\*\s*([^\n]*)',  # English
        r'\*\*Mức độ:\*\*\s*([^\n]*)'  # Vietnamese
    ]
    current_code_patterns = [
        r'\*\*Current Code:\*\*\s*```cpp(.*?)```',  # English
        r'\*\*Code hiện tại:\*\*\s*```cpp(.*?)```'  # Vietnamese
    ]
    fixed_code_patterns = [
        r'\*\*Fixed Code:\*\*\s*```cpp(.*?)```',  # English
        r'\*\*Code đã sửa:\*\*\s*```cpp(.*?)```'  # Vietnamese
    ]
    explanation_patterns = [
        r'\*\*Explanation:\*\*\s*([^\n]*)',  # English
        r'\*\*Giải thích:\*\*\s*([^\n]*)'  # Vietnamese
    ]

    # Tách các block vi phạm
    issues = re.split(r'(?=^## 🚨 (Issue|Vấn đề))', md_text, flags=re.MULTILINE)
    results = []
    for issue in issues:
        if not (issue.strip().startswith('## 🚨 Issue') or issue.strip().startswith('## 🚨 Vấn đề')):
            continue

        def extract(patterns, text, default=''):
            for pattern in patterns:
                m = re.search(pattern, text, re.DOTALL)
                if m:
                    return m.group(1).strip()
            return default

        issue_title = extract(issue_patterns, issue)
        # Loại bỏ số thứ tự ở đầu tiêu đề nếu có (ví dụ: "1: Magic Number" -> "Magic Number")
        issue_title = re.sub(r'^\d+\s*[:\-\.]?\s*', '', issue_title)
        rule = extract(rule_patterns, issue)
        location = extract(location_patterns, issue)
        severity = extract(severity_patterns, issue)
        current_code = extract(current_code_patterns, issue)
        fixed_code = extract(fixed_code_patterns, issue)
        explanation = extract(explanation_patterns, issue)
        results.append({
            'Issue': issue_title,
            'Rule': rule,
            'Location': location,
            'Severity': severity,
            'Current Code': current_code,
            'Fixed Code': fixed_code,
            'Explanation': explanation
        })
    return results

def render_code_with_lines(code, lang="cpp"):
    lines = code.splitlines() if code else []
    if not lines:
        return '<pre><code class="language-cpp"></code></pre>'
    line_numbers = ''.join(f'<span class="line-number" style="display:block;">{i+1}</span>' for i in range(len(lines)))
    code_html = '\n'.join(lines)
    return f'''
    <div style="display:flex;">
      <div style="background:#f6f8fa;color:#888;padding:6px 8px 6px 6px;font-family:monospace;font-size:13px;line-height:1.5;text-align:right;user-select:none;border-right:1px solid #e1e4e8;">
        {line_numbers}
      </div>
      <pre style="margin:0;flex:1;"><code class="language-{lang}">{code_html}</code></pre>
    </div>
    '''

def is_placeholder(v):
    return (
        v["Issue"].strip().startswith("[#]:") or
        v["Rule"].strip().startswith("[EXACT_RULE_NUMBER]") or
        v["Location"].strip().startswith("[function name") or
        v["Severity"].strip().startswith("[Critical") or
        v["Current Code"].strip() == "[problematic code]" or
        v["Fixed Code"].strip() == "[corrected code]" or
        v["Explanation"].strip() == "[Why this violates the rule and how fix works]"
    )

def write_html_table(violations, output_path):
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write('<!DOCTYPE html>\n<html><head><meta charset="utf-8"><title>LGEDV Report</title>\n')
        f.write('<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/github.min.css">\n')
        f.write('<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js"></script>\n')
        f.write('<script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/languages/cpp.min.js"></script>\n')
        f.write('<script>hljs.highlightAll();</script>\n')
        f.write('''
        <style>
        table{border-collapse:collapse;}
        th,td{border:1px solid #888;padding:6px;vertical-align:top;}
        pre{margin:0;}
        .code-col { max-width: 480px; min-width: 180px; }
        .code-col > div { max-width: 480px; overflow-x: auto; }
        .rule-col { max-width: 260px; min-width: 120px; overflow-x: auto; word-break: break-word; }
        .issue-col { max-width: 180px; min-width: 80px; overflow-x: auto; word-break: break-word; }
        .location-col { max-width: 340px; min-width: 140px; overflow-x: auto; word-break: break-word; }
        .explanation-col { max-width: 220px; min-width: 80px; overflow-x: auto; word-break: break-word; }
        </style>
        ''')
        f.write('</head><body>\n')
        f.write('<table>\n')
        f.write('<tr><th>Issue</th><th>Rule Violated</th><th>Location</th><th>Current Code</th><th>Fixed Code</th><th>Explanation</th></tr>\n')
        idx = 1
        for v in violations:
            if is_placeholder(v):
                continue
            f.write('<tr>')
            # Đánh số thứ tự tự động, loại bỏ số trong tiêu đề nếu có
            f.write(f'<td class="issue-col">{idx}. {v["Issue"]}</td>')
            f.write(f'<td class="rule-col">{v["Rule"]}</td>')
            f.write(f'<td class="location-col">{v["Location"]}</td>')
            f.write(f'<td class="code-col">{render_code_with_lines(v["Current Code"])}</td>')
            f.write(f'<td class="code-col">{render_code_with_lines(v["Fixed Code"])}</td>')
            f.write(f'<td class="explanation-col">{v["Explanation"]}</td>')
            f.write('</tr>\n')
            idx += 1
        f.write('</table>\n')
        f.write('</body></html>\n')

def print_usage():
    print("Usage: python report_http.py <input.md> [-o <output_file>]")
    sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) == 2:
        input_path = sys.argv[1]
        output_path = "output.html"
    elif len(sys.argv) == 4 and sys.argv[2] == "-o":
        input_path = sys.argv[1]
        output_path = sys.argv[3]
    else:
        print_usage()
    with open(input_path, 'r', encoding='utf-8') as f:
        md_text = f.read()
    violations = extract_violations(md_text)
    write_html_table(violations, output_path)