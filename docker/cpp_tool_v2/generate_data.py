%%writefile /content/misra_labeling_script.py
import subprocess
import re
import json
import random
import os
from pathlib import Path

# Chuẩn: MISRA C++:2008
MISRA_RULES = {
    "misra-cpp-5-0-1": {
        "label": "Rule 5-0-1",
        "description": "Cấm sử dụng bộ nhớ động (new/delete) [MISRA C++:2008 Rule 5-0-1]",
        "fix": lambda code, line: re.sub(r"int\*\s*(\w+)\s*=\s*new\s*int\s*;", r"int \g<1> = 0;", code)
    },
    "misra-cpp-5-0-2": {
        "label": "Rule 5-0-2",
        "description": "Giá trị biểu thức phải không phụ thuộc vào thứ tự đánh giá [MISRA C++:2008 Rule 5-0-2]",
        "fix": lambda code, line: re.sub(r"int\s+(\w+)\s*=\s*\w+\+\+\s*\+\s*\w+\s*;", r"int \g<1> = 0; // Tách biểu thức", code)
    },
    "misra-cpp-6-4-1": {
        "label": "Rule 6-4-1",
        "description": "Cấm sử dụng goto [MISRA C++:2008 Rule 6-4-1]",
        "fix": lambda code, line: re.sub(r"goto \w+;.*?\w+:.*?\n", "", code, flags=re.DOTALL)
    },
    "misra-cpp-8-4-1": {
        "label": "Rule 8-4-1",
        "description": "Biến phải được khởi tạo trước khi sử dụng [MISRA C++:2008 Rule 8-4-1]",
        "fix": lambda code, line: re.sub(r"(\bint\s+\w+\s*;)", r"\1 = 0;", code)
    },
    "misra-cpp-3-1-1": {
        "label": "Rule 3-1-1",
        "description": "Cấm sử dụng hàm không an toàn (strcpy, strcat, ...) [MISRA C++:2008 Rule 3-1-1]",
        "fix": lambda code, line: code.replace("strcpy", "std::strcpy")
    },
    "noViolation": {
        "label": "No violation",
        "description": "Không vi phạm MISRA C++:2008",
        "fix": lambda code, line: code
    }
}

# Mẫu mã C++ tổng hợp
VIOLATION_TEMPLATES_5_0_1 = [
    "void {func_name}() {{ int* {var_name} = new int; *{var_name} = {value}; }}",
    "void {func_name}() {{ int* {var_name} = new int[{value}]; }}",
]
NON_VIOLATION_TEMPLATES_5_0_1 = [
    "void {func_name}() {{ int {var_name} = {value}; }}",
    "void {func_name}() {{ {type} {var_name} = {value}; {var_name}++; }}",
]
VIOLATION_TEMPLATES_5_0_2 = [
    "void {func_name}() {{ int i = 0; int {var_name} = i++ + i; }}",
    "void {func_name}() {{ int j = {value}; int {var_name} = j-- + j; }}",
]
NON_VIOLATION_TEMPLATES_5_0_2 = [
    "void {func_name}() {{ int i = 0; i++; int {var_name} = i + i; }}",
    "void {func_name}() {{ int j = {value}; j--; int {var_name} = j + j; }}",
]
VIOLATION_TEMPLATES_3_1_1 = [
    "void {func_name}() {{ strcpy(dst, src); }}",
    "void {func_name}() {{ strcat(dst, src); }}",
]
NON_VIOLATION_TEMPLATES_3_1_1 = [
    "void {func_name}() {{ std::strcpy(dst, src); }}",
    "void {func_name}() {{ std::strcat(dst, src); }}",
]

def generate_random_name(prefix="func"):
    return f"{prefix}_{random.randint(1, 1000)}"

def generate_random_var_name():
    return f"var_{random.randint(1, 1000)}"

def generate_random_value():
    return random.randint(0, 100)

def generate_random_type():
    return random.choice(["int", "float", "double"])

def generate_synthetic_example(rule_id, is_violation):
    rule = next(r for r in MISRA_RULES.values() if r["label"] == rule_id)
    if rule["label"] == "Rule 5-0-1":
        template = random.choice(VIOLATION_TEMPLATES_5_0_1 if is_violation else NON_VIOLATION_TEMPLATES_5_0_1)
        code = template.format(
            func_name=generate_random_name(),
            var_name=generate_random_var_name(),
            value=generate_random_value(),
            type=generate_random_type()
        )
    elif rule["label"] == "Rule 5-0-2":
        template = random.choice(VIOLATION_TEMPLATES_5_0_2 if is_violation else NON_VIOLATION_TEMPLATES_5_0_2)
        code = template.format(
            func_name=generate_random_name(),
            var_name=generate_random_var_name(),
            value=generate_random_value()
        )
    elif rule["label"] == "Rule 3-1-1":
        template = random.choice(VIOLATION_TEMPLATES_3_1_1 if is_violation else NON_VIOLATION_TEMPLATES_3_1_1)
        code = template.format(
            func_name=generate_random_name()
        )
    else:
        code = f"void {generate_random_name()}() {{ int {generate_random_var_name()} = {generate_random_value()}; }}"
    fixed_code = rule["fix"](code, 1) if is_violation else code
    return {
        "code": code,
        "label": rule["label"],
        "description": rule["description"],
        "suggestion": fixed_code,
        "line_number": 1
    }

def run_clang_tidy(file_path):
    output_file = "clang_tidy_output.txt"
    cmd = f"clang-tidy {file_path} -checks='misra-*' -- > {output_file} 2>/dev/null"
    subprocess.run(cmd, shell=True)
    errors = []
    try:
        with open(output_file, 'r', encoding='utf-8') as f:
            content = f.read()
            pattern = r"(\w+\.cpp):(\d+):\d+:.*?\[([^\]]*misra-cpp-[^\]]*)\]"
            matches = re.findall(pattern, content)
            for file, line, error_id in matches:
                if error_id in MISRA_RULES:
                    errors.append((error_id, MISRA_RULES[error_id]["label"], int(line)))
        return errors
    except FileNotFoundError:
        return []
    finally:
        if os.path.exists(output_file):
            os.remove(output_file)

def label_code(file_path):
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            code = f.read().strip()
        errors = run_clang_tidy(file_path)
        dataset_entries = []
        if not errors:
            if "new " in code:
                fixed_code = re.sub(r"int\*\s*(\w+)\s*=\s*new\s*int\s*;", r"int \g<1> = 0;", code)
                dataset_entries.append({
                    "code": code,
                    "label": "Rule 5-0-1",
                    "description": "Cấm sử dụng bộ nhớ động (new/delete) [MISRA C++:2008 Rule 5-0-1]",
                    "suggestion": fixed_code,
                    "line_number": [i + 1 for i, line in enumerate(code.split('\n')) if "new " in line][0]
                })
            elif re.search(r"\w+\+\+\s*\+\s*\w+", code):
                fixed_code = re.sub(r"int\s+(\w+)\s*=\s*\w+\+\+\s*\+\s*\w+\s*;", r"int \g<1> = 0; // Tách biểu thức", code)
                dataset_entries.append({
                    "code": code,
                    "label": "Rule 5-0-2",
                    "description": "Giá trị biểu thức phải không phụ thuộc vào thứ tự đánh giá [MISRA C++:2008 Rule 5-0-2]",
                    "suggestion": fixed_code,
                    "line_number": [i + 1 for i, line in enumerate(code.split('\n')) if "++" in line][0]
                })
            elif "goto " in code:
                fixed_code = re.sub(r"goto \w+;.*?\w+:.*?\n", "", code, flags=re.DOTALL)
                dataset_entries.append({
                    "code": code,
                    "label": "Rule 6-4-1",
                    "description": "Cấm sử dụng goto [MISRA C++:2008 Rule 6-4-1]",
                    "suggestion": fixed_code,
                    "line_number": [i + 1 for i, line in enumerate(code.split('\n')) if "goto " in line][0]
                })
            elif "strcpy" in code or "strcat" in code:
                fixed_code = code.replace("strcpy", "std::strcpy").replace("strcat", "std::strcat")
                dataset_entries.append({
                    "code": code,
                    "label": "Rule 3-1-1",
                    "description": "Cấm sử dụng hàm không an toàn (strcpy, strcat, ...) [MISRA C++:2008 Rule 3-1-1]",
                    "suggestion": fixed_code,
                    "line_number": [i + 1 for i, line in enumerate(code.split('\n')) if "strcpy" in line or "strcat" in line][0]
                })
            else:
                dataset_entries.append({
                    "code": code,
                    "label": "No violation",
                    "description": "Không vi phạm MISRA C++:2008",
                    "suggestion": code,
                    "line_number": 0
                })
        else:
            for error_id, error_label, line_number in errors:
                fixed_code = MISRA_RULES[error_id]["fix"](code, line_number)
                dataset_entries.append({
                    "code": code,
                    "label": error_label,
                    "description": MISRA_RULES[error_id]["description"],
                    "suggestion": fixed_code,
                    "line_number": line_number
                })
        return dataset_entries
    except Exception as e:
        print(f"Error processing {file_path}: {e}")
        return []

def create_misra_dataset(num_synthetic=1000, input_dir="/content/cpp_files", output_file="/content/drive/MyDrive/misra_dataset.json"):
    dataset = []
    # Tạo dữ liệu tổng hợp
    rule_ids = ["Rule 5-0-1", "Rule 5-0-2", "Rule 3-1-1"]
    for _ in range(num_synthetic):
        rule_id = random.choice(rule_ids)
        is_violation = random.choice([True, False])
        example = generate_synthetic_example(rule_id, is_violation)
        dataset.append(example)
    # Xử lý file C++ thực tế
    for file_path in Path(input_dir).glob("*.cpp"):
        if file_path.is_file():
            entries = label_code(file_path)
            dataset.extend(entries)
    # Lưu dataset
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(dataset, f, indent=2, ensure_ascii=False)
    print(f"Đã tạo {len(dataset)} mẫu dữ liệu và lưu vào {output_file}")
    return dataset

def main():
    subprocess.run("apt-get update && apt-get install -y clang-tidy", shell=True, check=True)
    create_misra_dataset()

if __name__ == "__main__":
    main()