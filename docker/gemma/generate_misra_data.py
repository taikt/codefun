import json
import random
import clang.cindex
from pathlib import Path

# Danh sách quy tắc MISRA
MISRA_RULES = [
    {
        "rule_id": "MISRA C++ 3-1-1",
        "description": "Không sử dụng các đặc tả hàm inline trong định nghĩa hàm."
    },
    {
        "rule_id": "MISRA C++ 5-0-1",
        "description": "Biểu thức logic không được chứa toán tử && hoặc || ở cấp độ cao nhất."
    }
]

# Mẫu mã C++ cho MISRA C++ 3-1-1
VIOLATION_TEMPLATES_3_1_1 = [
    "inline void {func_name}() {{ int x = {value}; }}",
    "inline int {func_name}(int a) {{ return a + {value}; }}",
    "inline void {func_name}() {{ {type} y = {value}; y++; }}",
    "class {class_name} {{ inline void {func_name}() {{ int z = {value}; }} }};"
]

NON_VIOLATION_TEMPLATES_3_1_1 = [
    "void {func_name}() {{ int x = {value}; }}",
    "int {func_name}(int a) {{ return a + {value}; }}",
    "void {func_name}() {{ {type} y = {value}; y++; }}",
    "class {class_name} {{ void {func_name}() {{ int z = {value}; }} }};"
]

# Mẫu mã C++ cho MISRA C++ 5-0-1
VIOLATION_TEMPLATES_5_0_1 = [
    "if ({cond1} && {cond2}) {{ int x = {value}; }}",
    "if ({cond1} || {cond2}) {{ {type} y = {value}; }}",
    "while ({cond1} && {cond2}) {{ int z = {value}; z++; }}"
]

NON_VIOLATION_TEMPLATES_5_0_1 = [
    "if ({cond1}) {{ if ({cond2}) {{ int x = {value}; }} }}",
    "if ({cond1}) {{ int x = {value}; }} else if ({cond2}) {{ int x = {value}; }}",
    "while ({cond1}) {{ if ({cond2}) {{ int z = {value}; z++; }} }}"
]

# Hàm tạo tên và giá trị ngẫu nhiên
def generate_random_name(prefix="func"):
    return f"{prefix}_{random.randint(1, 1000)}"

def generate_random_value():
    return random.randint(0, 100)

def generate_random_type():
    return random.choice(["int", "float", "double", "char"])

def generate_random_condition():
    return random.choice(["x > 0", "y < 10", "a == b", "flag"])

# Hàm tạo mẫu dữ liệu tổng hợp
def generate_synthetic_example(rule_id, is_violation):
    rule = next(r for r in MISRA_RULES if r["rule_id"] == rule_id)
    if rule_id == "MISRA C++ 3-1-1":
        template = random.choice(VIOLATION_TEMPLATES_3_1_1 if is_violation else NON_VIOLATION_TEMPLATES_3_1_1)
        code = template.format(
            func_name=generate_random_name("func"),
            class_name=generate_random_name("Class"),
            value=generate_random_value(),
            type=generate_random_type()
        )
        suggestion = "Xóa từ khóa inline hoặc di chuyển khai báo inline vào tệp tiêu đề." if is_violation else ""
    elif rule_id == "MISRA C++ 5-0-1":
        template = random.choice(VIOLATION_TEMPLATES_5_0_1 if is_violation else NON_VIOLATION_TEMPLATES_5_0_1)
        code = template.format(
            cond1=generate_random_condition(),
            cond2=generate_random_condition(),
            value=generate_random_value(),
            type=generate_random_type()
        )
        suggestion = "Tách biểu thức logic thành các câu lệnh if lồng nhau." if is_violation else ""
    return {
        "code": code,
        "rule_id": rule["rule_id"],
        "violation": is_violation,
        "description": rule["description"],
        "suggestion": suggestion
    }

# Hàm trích xuất hàm từ mã C++ bằng libclang
def extract_functions(file_path):
    try:
        index = clang.cindex.Index.create()
        tu = index.parse(file_path, args=['-std=c++17'])
        functions = []
        
        def traverse(node):
            if node.kind == clang.cindex.CursorKind.FUNCTION_DECL:
                tokens = [t.spelling for t in node.get_tokens()]
                code = "".join(tokens)
                line = node.location.line
                functions.append({"code": code, "line": line})
            for child in node.get_children():
                traverse(child)
        
        traverse(tu.cursor)
        return functions
    except Exception as e:
        print(f"Error parsing {file_path}: {e}")
        return []

# Hàm gắn nhãn vi phạm MISRA
def label_function(code, rule_id):
    if rule_id == "MISRA C++ 3-1-1":
        is_violation = "inline" in code
        suggestion = "Xóa từ khóa inline hoặc di chuyển khai báo inline vào tệp tiêu đề." if is_violation else ""
    elif rule_id == "MISRA C++ 5-0-1":
        is_violation = "&&" in code or "||" in code
        suggestion = "Tách biểu thức logic thành các câu lệnh if lồng nhau." if is_violation else ""
    else:
        is_violation = False
        suggestion = ""
    rule = next(r for r in MISRA_RULES if r["rule_id"] == rule_id)
    return {
        "code": code,
        "rule_id": rule["rule_id"],
        "violation": is_violation,
        "description": rule["description"],
        "suggestion": suggestion
    }

# Hàm tạo tập dữ liệu
def create_misra_dataset(num_synthetic=1000, cpp_files=None, output_file="/app/output/misra_dataset.json"):
    dataset = []
    
    # Tạo dữ liệu tổng hợp
    for _ in range(num_synthetic):
        rule_id = random.choice([r["rule_id"] for r in MISRA_RULES])
        is_violation = random.choice([True, False])
        example = generate_synthetic_example(rule_id, is_violation)
        dataset.append(example)
    
    # Trích xuất và gắn nhãn từ tệp C++
    if cpp_files:
        for cpp_file in cpp_files:
            functions = extract_functions(cpp_file)
            for func in functions:
                for rule in MISRA_RULES:
                    labeled = label_function(func["code"], rule["rule_id"])
                    labeled["line"] = func["line"]
                    dataset.append(labeled)
    
    # Lưu dữ liệu
    output_path = Path(output_file)
    with output_path.open('w', encoding='utf-8') as f:
        json.dump(dataset, f, indent=2, ensure_ascii=False)
    
    print(f"Đã tạo {len(dataset)} mẫu dữ liệu và lưu vào {output_file}")

# Chạy chương trình
if __name__ == "__main__":
    cpp_files = ["/app/example.cpp"] if Path("/app/example.cpp").exists() else []
    create_misra_dataset(num_synthetic=1000, cpp_files=cpp_files, output_file="/app/output/misra_dataset.json")