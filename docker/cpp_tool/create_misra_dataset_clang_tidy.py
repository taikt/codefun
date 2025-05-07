import subprocess
import re
import json
import random
import os
from pathlib import Path

# Định nghĩa các quy tắc MISRA C++:2008
MISRA_RULES = {
    "misra-cpp-5-0-1": {
        "label": "Rule 5-0-1",
        "description": "Cấm sử dụng bộ nhớ động (new/delete) [MISRA C++:2008 Rule 5-0-1]",
        "fix": lambda code, line: re.sub(r"int\*\s*(\w+)\s*=\s*new\s*int(?:\[.*?\])?\s*;", r"int \g<1> = 0;", code)
    },
    "misra-cpp-5-0-2": {
        "label": "Rule 5-0-2",
        "description": "Giá trị biểu thức phải không phụ thuộc vào thứ tự đánh giá [MISRA C++:2008 Rule 5-0-2]",
        "fix": lambda code, line: re.sub(r"int\s+(\w+)\s*=\s*\w+(?:\+\+|--)\s*\+\s*\w+\s*;", r"int \g<1> = 0; // Tách biểu thức", code)
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
        "fix": lambda code, line: code.replace("strcpy", "std::strcpy").replace("strcat", "std::strcat")
    },
    "noViolation": {
        "label": "No violation",
        "description": "Không vi phạm MISRA C++:2008",
        "fix": lambda code, line: code
    }
}

# Mẫu mã C++ để tạo file
VIOLATION_SAMPLES = {
    "Rule 5-0-1": [
        # Mẫu ngắn
        """void func_{id}() {{ int* ptr = new int; *ptr = {value}; }}""",
        # Mẫu dài
        """#include <iostream>
        void func_{id}(int size) {{
            int* arr = new int[size];
            for (int i = 0; i < size; ++i) {{
                arr[i] = i * 3;
            }}
            std::cout << "Array: ";
            for (int i = 0; i < size; ++i) {{
                std::cout << arr[i] << " ";
            }}
            std::cout << std::endl;
            int sum = 0;
            for (int i = 0; i < size; ++i) {{
                sum += arr[i];
            }}
            std::cout << "Sum: " << sum << std::endl;
        }}"""
    ],
    "Rule 5-0-2": [
        """void func_{id}() {{ int i = 0; int x = i++ + i; }}""",
        """void func_{id}() {{ int j = {value}; int x = j-- + j; }}"""
    ],
    "Rule 3-1-1": [
        """#include <cstring>
        void func_{id}() {{ strcpy(dst, src); }}""",
        """#include <cstring>
        void func_{id}() {{ strcat(dst, src); }}"""
    ],
    "Rule 6-4-1": [
        """void func_{id}() {{ goto label; label: int x = {value}; }}"""
    ],
    "Rule 8-4-1": [
        """void func_{id}() {{ int x; x = {value}; }}"""
    ],
    "No violation": [
        """void func_{id}() {{ int x = {value}; x++; }}""",
        """#include <iostream>
        void func_{id}(int size) {{
            int arr[100];
            for (int i = 0; i < size; ++i) {{
                arr[i] = i * 3;
            }}
            std::cout << "Array: ";
            for (int i = 0; i < size; ++i) {{
                std::cout << arr[i] << " ";
            }}
            std::cout << std::endl;
            int sum = 0;
            for (int i = 0; i < size; ++i) {{
                sum += arr[i];
            }}
            std::cout << "Sum: " << sum << std::endl;
        }}"""
    ]
}

def generate_random_value():
    return random.randint(0, 100)

def create_sample_files(output_dir, num_samples=100):
    Path(output_dir).mkdir(exist_ok=True)
    sample_files = []
    
    for i in range(num_samples):
        rule_id = random.choice(list(VIOLATION_SAMPLES.keys()))
        sample_code = random.choice(VIOLATION_SAMPLES[rule_id])
        code = sample_code.format(id=i, value=generate_random_value())
        
        file_path = f"{output_dir}/sample_{i}.cpp"
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(code)
        sample_files.append(file_path)
    
    return sample_files

def run_clang_tidy(file_path):
    output_file = "clang_tidy_output.txt"
    cmd = f"clang-tidy {file_path} -checks='misra-cpp-*' -- > {output_file} 2>/dev/null"
    subprocess.run(cmd, shell=True)
    errors = []
    try:
        with open(output_file, 'r', encoding='utf-8') as f:
            content = f.read()
            pattern = r"(\w+\.cpp):(\d+):\d+:.*?\[([^\]]*misra-cpp-[^\]]*)\]"
            matches = re.findall(pattern, content)
            for _, line, error_id in matches:
                if error_id in MISRA_RULES:
                    errors.append((error_id, MISRA_RULES[error_id]["label"], int(line)))
        return errors
    except FileNotFoundError:
        return []
    finally:
        if os.path.exists(output_file):
            os.remove(output_file)

def create_misra_dataset(num_samples=100, output_dir="./cpp_samples", dataset_file="./misra_dataset.json"):
    dataset = []
    
    # Tạo file C++ mẫu
    sample_files = create_sample_files(output_dir, num_samples)
    
    # Phân tích từng file bằng Clang-Tidy
    for file_path in sample_files:
        with open(file_path, 'r', encoding='utf-8') as f:
            code = f.read().strip()
        
        errors = run_clang_tidy(file_path)
        if not errors:
            dataset.append({
                "code": code,
                "label": "No violation",
                "description": "Không vi phạm MISRA C++:2008",
                "suggestion": code,
                "line_number": 0
            })
        else:
            for error_id, error_label, line_number in errors:
                fixed_code = MISRA_RULES[error_id]["fix"](code, line_number)
                dataset.append({
                    "code": code,
                    "label": error_label,
                    "description": MISRA_RULES[error_id]["description"],
                    "suggestion": fixed_code,
                    "line_number": line_number
                })
        
        # Xóa file sau khi xử lý
        os.remove(file_path)
    
    # Lưu dataset
    with open(dataset_file, 'w', encoding='utf-8') as f:
        json.dump(dataset, f, indent=2, ensure_ascii=False)
    print(f"Đã tạo {len(dataset)} mẫu dữ liệu và lưu vào {dataset_file}")
    
    return dataset

def main():
    # Cài đặt Clang-Tidy
    subprocess.run("apt-get update && apt-get install -y clang-tidy", shell=True, check=True)
    
    # Tạo dataset
    create_misra_dataset(num_samples=200)

if __name__ == "__main__":
    main()